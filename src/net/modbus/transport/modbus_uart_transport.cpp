#include "esp_log.h"
#include <inttypes.h>

#include <driver/gpio.h>
#include <rom/ets_sys.h>

#include <net/modbus/modbus_defs.hpp>
#include <net/modbus/transport/modbus_uart_transport.hpp>
#include "modbus_uart_transport.hpp"

namespace speed
{
    namespace net
    {
        namespace modbus
        {
            static const char *TAG = "ModbusUartTransport";

            //
            // UartConfig implementation
            //

            uint32_t UartConfig::calculateInterFrameDelay() const
            {
                // T3.5 = 3.5 * (1 start + 8 data + 1 parity + 1 stop) * 1000ms / baud_rate
                uint32_t charTime = (1000 * 11) / baudRate;        // Time for one character in ms
                uint32_t interFrameDelayMs = (35 * charTime) / 10; // 3.5 characters worth of delay

                // Use a minimum default value if calculated value is too small
                return (interFrameDelayMs > ModbusConstants::DEFAULT_INTER_FRAME_DELAY) ? interFrameDelayMs : ModbusConstants::DEFAULT_INTER_FRAME_DELAY;
            }

            uint32_t UartConfig::calculateInterCharTimeout() const
            {
                // T1.5 = 1.5 * (1 start + 8 data + 1 parity + 1 stop) * 1000ms / baud_rate
                uint32_t charTime = (1000 * 11) / baudRate;         // Time for one character in ms
                uint32_t interCharTimeoutMs = (15 * charTime) / 10; // 1.5 characters worth of timeout

                // Use a minimum default value if calculated value is too small
                return (interCharTimeoutMs > 1) ? interCharTimeoutMs : 1;
            }

            bool UartConfig::applyConfig(uart_port_t uart_num, int tx_pin, int rx_pin) const
            {
                // Configure UART parameters
                uart_config_t uart_config = {
                    .baud_rate = baudRate,
                    .data_bits = dataBits,
                    .parity = parity,
                    .stop_bits = stopBits,
                    .flow_ctrl = flowControl,
                    .rx_flow_ctrl_thresh = 0};

                esp_err_t err = uart_param_config(uart_num, &uart_config);
                if (err != ESP_OK)
                {
                    ESP_LOGE(TAG, "Failed to configure UART parameters with baud rate %d", baudRate);
                    return false;
                }

                // Set UART pins
                err = uart_set_pin(uart_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
                if (err != ESP_OK)
                {
                    ESP_LOGE(TAG, "Failed to set UART pins");
                    return false;
                }

                // Install UART driver
                err = uart_driver_install(uart_num, rxBufferSize, txBufferSize, 0, NULL, 0);
                if (err != ESP_OK)
                {
                    ESP_LOGE(TAG, "Failed to install UART driver");
                    return false;
                }

                return true;
            }

            //
            // Rs485UartConfig implementation
            //

            bool Rs485UartConfig::applyConfig(uart_port_t uart_num, int tx_pin, int rx_pin) const
            {
                // First apply the base configuration
                if (!UartConfig::applyConfig(uart_num, tx_pin, rx_pin))
                {
                    return false;
                }

                // Configure RS-485 half-duplex mode
                esp_err_t err = uart_set_mode(uart_num, UART_MODE_RS485_HALF_DUPLEX);
                if (err != ESP_OK)
                {
                    ESP_LOGE(TAG, "Failed to set RS-485 half-duplex mode");
                    uart_driver_delete(uart_num);
                    return false;
                }

                // Set RTS pin if specified
                if (rtsPin != UART_PIN_NO_CHANGE)
                {
                    err = uart_set_pin(uart_num, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, rtsPin, UART_PIN_NO_CHANGE);
                    if (err != ESP_OK)
                    {
                        ESP_LOGE(TAG, "Failed to set RTS pin for RS-485");
                        uart_driver_delete(uart_num);
                        return false;
                    }

                    // Configure GPIO for RTS pin if needed for manual control
                    gpio_config_t io_conf = {};
                    io_conf.mode = GPIO_MODE_OUTPUT;
                    io_conf.pin_bit_mask = (1ULL << rtsPin);
                    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
                    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
                    io_conf.intr_type = GPIO_INTR_DISABLE;

                    gpio_config(&io_conf);

                    // Initialize to receive mode
                    gpio_set_level(static_cast<gpio_num_t>(rtsPin), rtsInvert ? 1 : 0);
                }

                ESP_LOGI(TAG, "Configured RS-485 mode with RTS pin %d", rtsPin);
                return true;
            }

            void Rs485UartConfig::beforeTransmit(uart_port_t uart_num) const
            {
                if (rtsPin != UART_PIN_NO_CHANGE)
                {
                    gpio_set_level(static_cast<gpio_num_t>(rtsPin), rtsInvert ? 0 : 1);
                }
            }

            void Rs485UartConfig::afterTransmit(uart_port_t uart_num) const
            {
                if (rtsPin != UART_PIN_NO_CHANGE)
                {
                    // Wait for turnaround delay if configured
                    vTaskDelay(pdMS_TO_TICKS(turnaroundDelayMs));

                    // Set RTS for reception
                    gpio_set_level(static_cast<gpio_num_t>(rtsPin), rtsInvert ? 1 : 0);

                    // Additional delay after switching to receive mode if configured
                    if (txDelayMs > 0)
                    {
                        ets_delay_us(txDelayMs * 1000);
                    }
                }
            }

            //
            // RtuUartConfig implementation
            //

            uint32_t RtuUartConfig::calculateInterFrameDelay() const
            {
                uint32_t calculatedDelay = UartConfig::calculateInterFrameDelay();

                // Use the larger of the calculated delay or the configured delay
                return (calculatedDelay > interFrameDelayMs) ? calculatedDelay : interFrameDelayMs;
            }

            uint32_t RtuUartConfig::calculateInterCharTimeout() const
            {
                uint32_t calculatedTimeout = UartConfig::calculateInterCharTimeout();

                // Use the larger of the calculated timeout or the configured timeout
                return (calculatedTimeout > interCharTimeoutMs) ? calculatedTimeout : interCharTimeoutMs;
            }

            //
            // ModbusUartTransport implementation
            //

            ModbusUartTransport::ModbusUartTransport(uart_port_t uart_num,
                                                     int tx_pin, int rx_pin,
                                                     const std::shared_ptr<UartConfig> &config)
                : _uart_num(uart_num),
                  _tx_pin(tx_pin),
                  _rx_pin(rx_pin),
                  _config(config)
            {
            }

            ModbusUartTransport::~ModbusUartTransport()

            {
                stop();
            }

            bool ModbusUartTransport::begin()
            {
                if (_initialized)
                {
                    return true;
                }

                if (!_config)
                {
                    ESP_LOGE(TAG, "No configuration provided");
                    return false;
                }

                if (!_config->applyConfig(_uart_num, _tx_pin, _rx_pin))
                {
                    ESP_LOGE(TAG, "Failed to apply configuration");
                    return false;
                }

                _initialized = true;
                return true;
            }

            void ModbusUartTransport::stop()
            {
                if (_initialized)
                {
                    uart_driver_delete(_uart_num);
                    _initialized = false;
                }
            }

            bool ModbusUartTransport::send(const uint8_t *data, size_t length)
            {
                if (!_initialized)
                {
                    return false;
                }

                // Wait for inter-frame delay
                uint32_t interFrameDelayMs = _config->calculateInterFrameDelay();
                vTaskDelay(pdMS_TO_TICKS(interFrameDelayMs));

                // Let the config prepare for transmission
                _config->beforeTransmit(_uart_num);

                // Send data
                int written = uart_write_bytes(_uart_num, data, length);
                if (written != length)
                {
                    ESP_LOGE(TAG, "Failed to write UART data: %d/%d bytes written", written, length);
                    return false;
                }

                // Wait for transmission to complete
                uart_wait_tx_done(_uart_num, pdMS_TO_TICKS(1000));

                // Let the config handle post-transmission steps
                _config->afterTransmit(_uart_num);

                return true;
            }

            bool ModbusUartTransport::receive(uint8_t *buffer, size_t expected_length, uint32_t timeout_ms)
            {
                if (!_initialized)
                {
                    return false;
                }

                size_t received = 0;
                uint32_t startTime = xTaskGetTickCount();
                uint32_t interCharTimeoutMs = _config->calculateInterCharTimeout();

                while (received < expected_length)
                {
                    int len = uart_read_bytes(_uart_num, buffer + received, expected_length - received,
                                              pdMS_TO_TICKS(interCharTimeoutMs));

                    if (len < 0)
                    {
                        ESP_LOGE(TAG, "UART read error");
                        return false;
                    }

                    if (len == 0)
                    {
                        // Check for overall timeout
                        if ((xTaskGetTickCount() - startTime) > pdMS_TO_TICKS(timeout_ms))
                        {
                            ESP_LOGW(TAG, "Receive timeout after %d bytes", received);
                            return false;
                        }

                        // If we've already received some data but nothing new came in,
                        // this might be the end of the message
                        if (received > 0)
                        {
                            // Wait for inter-character timeout to see if more data arrives
                            vTaskDelay(pdMS_TO_TICKS(1));
                            continue;
                        }
                    }

                    received += len;
                }

                // Wait for inter-frame delay to ensure complete message
                uint32_t interFrameDelayMs = _config->calculateInterFrameDelay();
                vTaskDelay(pdMS_TO_TICKS(interFrameDelayMs));

                return true;
            }

            void ModbusUartTransport::flush()
            {
                if (_initialized)
                {
                    uart_flush(_uart_num);
                }
            }
        } // namespace modbus
    } // namespace net
} // namespace speed
#include "esp_log.h"
#include <inttypes.h>

#include <driver/gpio.h>
#include <rom/ets_sys.h>

#include <net/modbus/modbus_defs.hpp>
#include <net/modbus/transport/modbus_uart_transport.hpp>

namespace speed
{
    namespace net
    {
        namespace modbus
        {

            static const char *TAG = "ModbusUartTransport";

            ModbusUartTransport::ModbusUartTransport(uart_port_t uart_num, int tx_pin, int rx_pin, int rts_pin, int baud_rate)
                : _uart_num(uart_num), _tx_pin(tx_pin), _rx_pin(rx_pin), _rts_pin(rts_pin),
                  _baud_rate(baud_rate), _initialized(false) {}

            ModbusUartTransport::~ModbusUartTransport()
            {
                stop();
            }

            bool ModbusUartTransport::begin()
            {
                uart_config_t uart_config = {
                    .baud_rate = _baud_rate,
                    .data_bits = UART_DATA_8_BITS,
                    .parity = UART_PARITY_DISABLE,
                    .stop_bits = UART_STOP_BITS_1,
                    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

                esp_err_t err = uart_param_config(_uart_num, &uart_config);
                if (err != ESP_OK)
                {
                    ESP_LOGE(TAG, "Failed to configure UART parameters with baud rate %d", _baud_rate);
                    return false;
                }

                err = uart_set_pin(_uart_num, _tx_pin, _rx_pin, _rts_pin, UART_PIN_NO_CHANGE);
                if (err != ESP_OK)
                {
                    ESP_LOGE(TAG, "Failed to set UART pins");
                    return false;
                }

                err = uart_driver_install(_uart_num, MAX_FRAME_SIZE * 2, 0, 0, NULL, 0);
                if (err != ESP_OK)
                {
                    ESP_LOGE(TAG, "Failed to install UART driver");
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
                    return false;

                // Calculate inter-frame delay based on baud rate
                // T3.5 = 3.5 * 11 bits * 1000ms / baud_rate
                uint32_t interFrameDelayMs = (35 * 11 * 1000) / (_baud_rate * 10);
                if (interFrameDelayMs < ModbusConstants::DEFAULT_INTER_FRAME_DELAY)
                {
                    interFrameDelayMs = ModbusConstants::DEFAULT_INTER_FRAME_DELAY;
                }

                // Wait for any previous transmission to complete
                vTaskDelay(pdMS_TO_TICKS(interFrameDelayMs));

                // Set RTS high (enable transmitter)
                if (_rts_pin != UART_PIN_NO_CHANGE)
                {
                    gpio_set_level(static_cast<gpio_num_t>(_rts_pin), 1);
                }

                // Send data
                int written = uart_write_bytes(_uart_num, data, length);
                if (written != length)
                {
                    ESP_LOGE(TAG, "Failed to write UART data: %d/%d bytes written", written, length);
                    return false;
                }

                // Wait for transmission to complete
                uart_wait_tx_done(_uart_num, pdMS_TO_TICKS(1000));

                // Set RTS low (enable receiver)
                if (_rts_pin != UART_PIN_NO_CHANGE)
                {
                    gpio_set_level(static_cast<gpio_num_t>(_rts_pin), 0);
                    // Add a small delay to let the RS-485 driver settle
                    ets_delay_us(50);
                }

                return true;
            }

            bool ModbusUartTransport::receive(uint8_t *buffer, size_t expected_length, uint32_t timeout_ms)
            {
                if (!_initialized)
                    return false;

                size_t received = 0;
                uint32_t startTime = xTaskGetTickCount();

                while (received < expected_length)
                {
                    int len = uart_read_bytes(_uart_num, buffer + received, expected_length - received,
                                              pdMS_TO_TICKS(timeout_ms));

                    if (len < 0)
                    {
                        ESP_LOGE(TAG, "UART read error");
                        return false;
                    }

                    if (len == 0)
                    {
                        // Check for timeout
                        if ((xTaskGetTickCount() - startTime) > pdMS_TO_TICKS(timeout_ms))
                        {
                            ESP_LOGW(TAG, "Receive timeout");
                            return false;
                        }
                        vTaskDelay(pdMS_TO_TICKS(1));
                        continue;
                    }

                    received += len;
                }

                // Wait for inter-frame delay to ensure complete message
                uint32_t interFrameDelayMs = (35 * 11 * 1000) / (_baud_rate * 10);
                if (interFrameDelayMs < ModbusConstants::DEFAULT_INTER_FRAME_DELAY)
                {
                    interFrameDelayMs = ModbusConstants::DEFAULT_INTER_FRAME_DELAY;
                }
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
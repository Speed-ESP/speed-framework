#pragma once

#include "driver/uart.h"

#include <net/modbus/transport/modbus_transport.hpp>
#include <net/modbus/modbus_defs.hpp>
#include <net/modbus/packager/modbus_packager_factory.hpp>

namespace speed
{
    namespace net
    {
        namespace modbus
        {
            /**
             * Base UART configuration for Modbus RTU
             */
            class UartConfig 
            {
            public:
                UartConfig() = default;
                virtual ~UartConfig() = default;

                // Common parameters for all UART configurations
                int baudRate{ModbusConstants::DEFAULT_BAUD_RATE};    // Baud rate
                uart_word_length_t dataBits{UART_DATA_8_BITS};       // Data bits
                uart_parity_t parity{UART_PARITY_DISABLE};           // Parity
                uart_stop_bits_t stopBits{UART_STOP_BITS_1};         // Stop bits
                uart_hw_flowcontrol_t flowControl{UART_HW_FLOWCTRL_DISABLE}; // Flow control
                int rxBufferSize{1024};                              // RX buffer size
                int txBufferSize{1024};                              // TX buffer size
                
                /**
                 * Calculate inter-frame delay based on baud rate
                 * 
                 * @return Inter-frame delay in milliseconds
                 */
                virtual uint32_t calculateInterFrameDelay() const;
                
                /**
                 * Calculate inter-character timeout based on baud rate
                 * 
                 * @return Inter-character timeout in milliseconds
                 */
                virtual uint32_t calculateInterCharTimeout() const;
                
                /**
                 * Apply configuration to UART
                 * 
                 * @param uart_num UART port number
                 * @param tx_pin TX pin number
                 * @param rx_pin RX pin number
                 * @return True if successful, false otherwise
                 */
                virtual bool applyConfig(uart_port_t uart_num, int tx_pin, int rx_pin) const;

                /**
                 * Called before data transmission
                 * Allows configuration-specific pre-transmission steps
                 * 
                 * @param uart_num UART port number
                 */
                virtual void beforeTransmit(uart_port_t uart_num) const {};
                
                /**
                 * Called after data transmission
                 * Allows configuration-specific post-transmission steps
                 * 
                 * @param uart_num UART port number
                 */
                virtual void afterTransmit(uart_port_t uart_num) const {};
            };

            /**
             * RS-485 specific configuration for UART communication
             */
            class Rs485UartConfig : public UartConfig
            {
            public:
                Rs485UartConfig() : UartConfig() {
                    // Default RS-485 values
                }
                
                // RS-485 specific parameters
                int rtsPin{UART_PIN_NO_CHANGE};           // GPIO pin number for RTS control
                bool rtsInvert{false};                    // Invert RTS signal (high=receive, low=transmit)
                uint32_t txDelayMs{0};                    // Additional delay after TX before switching to RX
                uint32_t turnaroundDelayMs{10};           // Delay between TX and RX (ms)
                
                bool applyConfig(uart_port_t uart_num, int tx_pin, int rx_pin) const override;
                void beforeTransmit(uart_port_t uart_num) const override;
                void afterTransmit(uart_port_t uart_num) const override;
            };

            /**
             * RTU specific timing configuration for UART communication
             */
            class RtuUartConfig : public UartConfig
            {
            public:
                RtuUartConfig() : UartConfig() {
                    // Default RTU values
                }
                
                // RTU specific parameters
                uint32_t interFrameDelayMs{ModbusConstants::DEFAULT_INTER_FRAME_DELAY}; 
                uint32_t interCharTimeoutMs{2};
                
                uint32_t calculateInterFrameDelay() const override;
                uint32_t calculateInterCharTimeout() const override;
            };

            class ModbusUartTransport : public ModbusTransport
            {
            public:
                /**
                 * Create a new ModbusUartTransport with a specific configuration
                 * The only public constructor to enforce proper configuration
                 * 
                 * @param uart_num UART port number
                 * @param tx_pin TX pin number
                 * @param rx_pin RX pin number
                 * @param config UART configuration
                 */
                ModbusUartTransport(uart_port_t uart_num, int tx_pin, int rx_pin, 
                                    const std::shared_ptr<UartConfig>& config);

                ~ModbusUartTransport();

                bool begin() override;
                void stop() override;
                bool isConnected() override { return _initialized; }

                bool send(const uint8_t *data, size_t length) override;
                bool receive(uint8_t *buffer, size_t expected_length, uint32_t timeout_ms) override;
                
                void flush() override;
                
                // Frame length calculation methods
                size_t getHeaderSize() const override { return ModbusConstants::RTU_HEADER_SIZE; }
                size_t getFooterSize() const override { return ModbusConstants::RTU_CRC_SIZE; }
                
                size_t calculateFrameLength(size_t pduLength) const override { 
                    return getHeaderSize() + pduLength + getFooterSize();
                }
                
                size_t getExceptionResponseLength() const override { 
                    return ModbusConstants::RTU_EXCEPTION_LENGTH;
                }
                
                // Get the default packager for UART transport (RTU)
                std::shared_ptr<ModbusPackager> getDefaultPackager() const override {
                    return ModbusPackagerFactory::createRtuPackager();
                }

            private:
                // UART parameters
                uart_port_t _uart_num;
                int _tx_pin;
                int _rx_pin;
                bool _initialized;
                // Configuration
                std::shared_ptr<UartConfig> _config;
            };

        } // namespace modbus
    } // namespace net
} // namespace speed
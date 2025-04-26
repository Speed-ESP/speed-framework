#pragma once

#include "driver/uart.h"

#include <net/modbus/transport/modbus_transport.hpp>
#include <net/modbus/modbus_defs.hpp>

namespace speed
{
    namespace net
    {
        namespace modbus
        {

            class ModbusUartTransport : public ModbusTransport
            {
            public:
                ModbusUartTransport(uart_port_t uart_num, int tx_pin, int rx_pin, int rts_pin, int baud_rate);
                ~ModbusUartTransport();

                bool begin() override;
                void stop() override;
                bool isConnected() override { return _initialized; }

                bool send(const uint8_t *data, size_t length) override;
                bool receive(uint8_t *buffer, size_t expected_length, uint32_t timeout_ms) override;
                void flush() override;
                ModbusTransportType getType() const override { return ModbusTransportType::RTU; }
                
                // Frame length calculation methods
                size_t getHeaderSize() const override { return ModbusConstants::RTU_HEADER_SIZE; }
                size_t getFooterSize() const override { return ModbusConstants::RTU_CRC_SIZE; }
                
                size_t calculateFrameLength(size_t pduLength) const override { 
                    return getHeaderSize() + pduLength + getFooterSize();
                }
                
                size_t getExceptionResponseLength() const override { 
                    return ModbusConstants::RTU_EXCEPTION_LENGTH;
                }

            private:
                uart_port_t _uart_num;
                int _tx_pin;
                int _rx_pin;
                int _rts_pin;
                int _baud_rate;
                bool _initialized;
            };

        } // namespace modbus
    } // namespace net
} // namespace speed
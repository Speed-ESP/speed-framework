#pragma once


#include <string>
#include <mutex>
#include <atomic>
#include <net/modbus/transport/modbus_transport.hpp>
#include <net/modbus/modbus_defs.hpp>
#include "lwip/sockets.h"
namespace speed
{
    namespace net
    {
        namespace modbus
        {

            struct ModbusTcpHeader
            {
                uint16_t transactionId;
                uint16_t protocolId; // Always 0 for Modbus/TCP
                uint16_t length;     // Number of bytes following
                uint8_t unitId;      // Slave address
            };

            class ModbusTcpTransport : public ModbusTransport
            {
            public:
                ModbusTcpTransport(const std::string &host, uint16_t port);
                ~ModbusTcpTransport();

                bool begin() override;
                void stop() override;
                bool isConnected() override;

                bool send(const uint8_t *data, size_t length) override;
                bool receive(uint8_t *buffer, size_t expected_length, uint32_t timeout_ms) override;
                void flush() override;
                ModbusTransportType getType() const override { return ModbusTransportType::TCP; }

                // Frame length calculation methods
                size_t getHeaderSize() const override { return ModbusConstants::TCP_HEADER_SIZE; }
                size_t getFooterSize() const override { return 0; } // TCP doesn't use CRC
                
                size_t calculateFrameLength(size_t pduLength) const override { 
                    return getHeaderSize() + pduLength;
                }
                
                size_t getExceptionResponseLength() const override { 
                    return ModbusConstants::TCP_EXCEPTION_LENGTH;
                }

                // TCP-specific methods
                void setKeepAlive(bool enabled, uint32_t idle = 7200, uint32_t interval = 75, uint32_t count = 9);
                void setConnectionTimeout(uint32_t timeout_ms);

            private:
                bool connect();
                void disconnect();
                bool reconnect();
                bool sendWithHeader(const uint8_t *data, size_t length);
                bool receiveWithHeader(uint8_t *buffer, size_t &length, uint32_t timeout_ms);
                bool setSocketOptions();

                std::string _host;
                uint16_t _port;
                int _socket;
                bool _connected;
                struct sockaddr_in _server_addr;
                std::mutex _sendMutex;
                std::mutex _receiveMutex;
                std::atomic<uint16_t> _nextTransactionId{0};

                // Socket options
                bool _keepAliveEnabled{false};
                uint32_t _keepAliveIdle{7200};
                uint32_t _keepAliveInterval{75};
                uint32_t _keepAliveCount{9};
                uint32_t _connectionTimeout{5000};
            };

        } // namespace modbus
    } // namespace net
} // namespace speed
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include <cstring>
#include <net/modbus/modbus_defs.hpp>

#include <net/modbus/transport/modbus_tcp_transport.hpp>

namespace speed
{
    namespace net
    {
        namespace modbus
        {

            static const char *TAG = "ModbusTcpTransport";

            ModbusTcpTransport::ModbusTcpTransport(const std::string &host, uint16_t port)
                : _host(host), _port(port), _socket(-1), _connected(false)
            {
                memset(&_server_addr, 0, sizeof(_server_addr));
            }

            ModbusTcpTransport::~ModbusTcpTransport()
            {
                stop();
            }

            bool ModbusTcpTransport::begin()
            {
                return connect();
            }

            void ModbusTcpTransport::stop()
            {
                disconnect();
            }

            bool ModbusTcpTransport::isConnected()
            {
                if (!_connected)
                    return false;

                // Check if connection is still alive
                char dummy;
                int result = recv(_socket, &dummy, 1, MSG_PEEK | MSG_DONTWAIT);
                if (result == 0 || (result < 0 && errno != EAGAIN && errno != EWOULDBLOCK))
                {
                    ESP_LOGW(TAG, "Connection lost, attempting reconnection");
                    disconnect();
                    return reconnect();
                }

                return _connected;
            }

            void ModbusTcpTransport::setKeepAlive(bool enabled, uint32_t idle, uint32_t interval, uint32_t count)
            {
                _keepAliveEnabled = enabled;
                _keepAliveIdle = idle;
                _keepAliveInterval = interval;
                _keepAliveCount = count;

                if (_connected)
                {
                    setSocketOptions();
                }
            }

            void ModbusTcpTransport::setConnectionTimeout(uint32_t timeout_ms)
            {
                _connectionTimeout = timeout_ms;
                if (_connected)
                {
                    struct timeval timeout;
                    timeout.tv_sec = timeout_ms / 1000;
                    timeout.tv_usec = (timeout_ms % 1000) * 1000;
                    setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
                    setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
                }
            }

            bool ModbusTcpTransport::connect()
            {
                if (_connected)
                    return true;

                _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (_socket < 0)
                {
                    ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
                    return false;
                }

                _server_addr.sin_family = AF_INET;
                _server_addr.sin_port = htons(_port);
                inet_pton(AF_INET, _host.c_str(), &_server_addr.sin_addr.s_addr);

                // Set socket options
                if (!setSocketOptions())
                {
                    close(_socket);
                    _socket = -1;
                    return false;
                }

                if (::connect(_socket, (struct sockaddr *)&_server_addr, sizeof(_server_addr)) != 0)
                {
                    ESP_LOGE(TAG, "Socket connection failed: errno %d", errno);
                    close(_socket);
                    _socket = -1;
                    return false;
                }

                _connected = true;
                ESP_LOGI(TAG, "Successfully connected to %s:%d", _host.c_str(), _port);
                return true;
            }

            bool ModbusTcpTransport::setSocketOptions()
            {
                struct timeval timeout;
                timeout.tv_sec = _connectionTimeout / 1000;
                timeout.tv_usec = (_connectionTimeout % 1000) * 1000;

                if (setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
                {
                    ESP_LOGE(TAG, "Failed to set receive timeout");
                    return false;
                }

                if (setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0)
                {
                    ESP_LOGE(TAG, "Failed to set send timeout");
                    return false;
                }

                if (_keepAliveEnabled)
                {
                    int keepAlive = 1;
                    if (setsockopt(_socket, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(keepAlive)) < 0)
                    {
                        ESP_LOGE(TAG, "Failed to set keep-alive");
                        return false;
                    }

                    setsockopt(_socket, IPPROTO_TCP, TCP_KEEPIDLE, &_keepAliveIdle, sizeof(_keepAliveIdle));
                    setsockopt(_socket, IPPROTO_TCP, TCP_KEEPINTVL, &_keepAliveInterval, sizeof(_keepAliveInterval));
                    setsockopt(_socket, IPPROTO_TCP, TCP_KEEPCNT, &_keepAliveCount, sizeof(_keepAliveCount));
                }

                return true;
            }

            void ModbusTcpTransport::disconnect()
            {
                if (_socket >= 0)
                {
                    close(_socket);
                    _socket = -1;
                }
                _connected = false;
            }

            bool ModbusTcpTransport::reconnect()
            {
                disconnect();
                return connect();
            }

            bool ModbusTcpTransport::send(const uint8_t *data, size_t length)
            {
                if (!_connected && !reconnect())
                    return false;

                std::lock_guard<std::mutex> lock(_sendMutex);
                return sendWithHeader(data, length);
            }

            bool ModbusTcpTransport::sendWithHeader(const uint8_t *data, size_t length)
            {
                std::lock_guard<std::mutex> lock(_sendMutex);

                ModbusTcpHeader header{
                    .transactionId = _nextTransactionId.fetch_add(1, std::memory_order_relaxed),
                    .protocolId = ModbusConstants::MODBUS_PROTOCOL_ID,
                    .length = static_cast<uint16_t>(length + 1), // +1 for unit ID
                    .unitId = data[0]                            // First byte is slave address
                };

                // Reset transaction ID if it wraps around to 0
                if (_nextTransactionId.load(std::memory_order_relaxed) == 0)
                {
                    _nextTransactionId.store(1, std::memory_order_relaxed);
                }

                // Send header
                if (send(reinterpret_cast<uint8_t *>(&header), sizeof(header)) < 0)
                {
                    ESP_LOGE(TAG, "Failed to send TCP header");
                    return false;
                }

                // Send PDU (skip first byte as it's already in header.unitId)
                if (send(data + 1, length - 1) < 0)
                {
                    ESP_LOGE(TAG, "Failed to send PDU");
                    return false;
                }

                return true;
            }

            bool ModbusTcpTransport::receive(uint8_t *buffer, size_t expected_length, uint32_t timeout_ms)
            {
                if (!_connected)
                    return false;

                std::lock_guard<std::mutex> lock(_receiveMutex);
                size_t actualLength;
                if (!receiveWithHeader(buffer, actualLength, timeout_ms))
                {
                    return false;
                }

                return actualLength == expected_length;
            }

            bool ModbusTcpTransport::receiveWithHeader(uint8_t *buffer, size_t &length, uint32_t timeout_ms)
            {
                std::lock_guard<std::mutex> lock(_receiveMutex);

                ModbusTcpHeader header;
                if (receive(reinterpret_cast<uint8_t *>(&header), sizeof(header), timeout_ms) < 0)
                {
                    ESP_LOGE(TAG, "Failed to receive TCP header");
                    return false;
                }

                // Validate protocol ID
                if (header.protocolId != ModbusConstants::MODBUS_PROTOCOL_ID)
                {
                    ESP_LOGE(TAG, "Invalid protocol ID: %d", header.protocolId);
                    return false;
                }

                // Calculate PDU length
                size_t pduLength = header.length - 1; // -1 for unit ID
                if (pduLength > length)
                {
                    ESP_LOGE(TAG, "Response too large for buffer");
                    return false;
                }

                // Copy unit ID to first byte of buffer
                buffer[0] = header.unitId;

                // Receive PDU
                if (receive(buffer + 1, pduLength, timeout_ms) < 0)
                {
                    ESP_LOGE(TAG, "Failed to receive PDU");
                    return false;
                }

                length = pduLength + 1; // +1 for unit ID
                return true;
            }

            void ModbusTcpTransport::flush()
            {
                if (!_connected)
                    return;

                std::lock_guard<std::mutex> lock(_receiveMutex);

                int flags = fcntl(_socket, F_GETFL, 0);
                fcntl(_socket, F_SETFL, flags | O_NONBLOCK);

                uint8_t buffer[256];
                while (recv(_socket, buffer, sizeof(buffer), 0) > 0)
                {
                    // Keep reading until no more data
                }

                fcntl(_socket, F_SETFL, flags);
            }

        } // namespace modbus
    } // namespace net
} // namespace speed
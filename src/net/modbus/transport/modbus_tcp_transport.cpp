#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include <cstring>
#include <inttypes.h>
#include <net/modbus/modbus_defs.hpp>
#include <vector>

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
                ESP_LOGD(TAG, "Connecting modbus tcp socket");
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
                ESP_LOGI(TAG, "Successfully connected to %s:%" PRIu16, _host.c_str(), _port);
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
                {
                    ESP_LOGD(TAG, "Socket not connected wen trying to send");
                    return false;
                }

                std::lock_guard<std::mutex> lock(_sendMutex);
                return sendModbusTcpPacket(data, length);
            }

            bool ModbusTcpTransport::sendModbusTcpPacket(const uint8_t *data, size_t length)
            {
                ESP_LOGD(TAG, "Sending data to: %s", _host.c_str());
                ESP_LOG_BUFFER_HEXDUMP(TAG, data, length, ESP_LOG_DEBUG);
                // Send the entire data buffer without header
                if (!sendToSocket(data, length))
                {
                    ESP_LOGE(TAG, "Failed to send data");
                    return false;
                }
                return true;
            }

            bool ModbusTcpTransport::receive(uint8_t *buffer, size_t expected_length, uint32_t timeout_ms)
            {
                if (!_connected && !reconnect())
                    return false;

                std::lock_guard<std::mutex> lock(_receiveMutex);
                size_t actualLength = expected_length;
                if (!receiveModbusTcpPackage(buffer, actualLength, timeout_ms))
                {
                    return false;
                }

                // When using headers, verify that the actual length matches what we expected
                return actualLength == expected_length;
            }

            bool ModbusTcpTransport::receiveModbusTcpPackage(uint8_t *buffer, size_t &length, uint32_t timeout_ms)
            {

                if (!receiveFromSocket(buffer, length, timeout_ms))
                {
                    ESP_LOGE(TAG, "Failed to receive data without header");
                    return false;
                }
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

            bool ModbusTcpTransport::sendToSocket(const uint8_t *data, size_t length)
            {
                if (!_connected)
                {
                    return false;
                }

                size_t bytes_sent = 0;
                while (bytes_sent < length)
                {
                    int ret = ::send(_socket, data + bytes_sent, length - bytes_sent, 0);
                    if (ret < 0)
                    {
                        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                        return false;
                    }
                    bytes_sent += ret;
                }
                return true;
            }

            bool ModbusTcpTransport::receiveFromSocket(uint8_t *buffer, size_t length, uint32_t timeout_ms)
            {
                if (!_connected)
                {
                    return false;
                }

                ESP_LOGD(TAG,"Starting receve data from %s", _host.c_str());

                struct timeval original_timeout;
                socklen_t len = sizeof(original_timeout);
                // Save original timeout
                if (getsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, &original_timeout, &len) < 0)
                {
                    ESP_LOGW(TAG, "Failed to get socket timeout, using default");
                }

                // Set new timeout if different from default
                if (timeout_ms != _connectionTimeout)
                {
                    struct timeval timeout;
                    timeout.tv_sec = timeout_ms / 1000;
                    timeout.tv_usec = (timeout_ms % 1000) * 1000;
                    if (setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
                    {
                        ESP_LOGW(TAG, "Failed to set receive timeout for this operation");
                    }
                }

                size_t bytes_received = 0;
                while (bytes_received < length)
                {
                    int ret = ::recv(_socket, buffer + bytes_received, length - bytes_received, 0);
                    if (ret < 0)
                    {
                        ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
                        // Restore original timeout if we changed it
                        if (timeout_ms != _connectionTimeout)
                        {
                            setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, &original_timeout, len);
                        }
                        return false;
                    }
                    else if (ret == 0)
                    {
                        ESP_LOGW(TAG, "Connection closed by peer");
                        // Restore original timeout if we changed it
                        if (timeout_ms != _connectionTimeout)
                        {
                            setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, &original_timeout, len);
                        }
                        _connected = false;
                        return false;
                    }
                    bytes_received += ret;
                }

                // Restore original timeout if we changed it
                if (timeout_ms != _connectionTimeout)
                {
                    setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, &original_timeout, len);
                }

                ESP_LOGD(TAG, "Data from %s received!", _host.c_str());
                ESP_LOG_BUFFER_HEX_LEVEL(TAG, buffer, length, ESP_LOG_DEBUG);
                return true;
            }

        } // namespace modbus
    } // namespace net
} // namespace speed
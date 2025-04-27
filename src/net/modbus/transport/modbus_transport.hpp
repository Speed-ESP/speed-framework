#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace speed
{
    namespace net
    {
        namespace modbus
        {
            // Forward declaration
            class ModbusPackager;

            class ModbusTransport
            {
            public:
                virtual ~ModbusTransport() = default;

                virtual bool begin() = 0;
                virtual void stop() = 0;
                virtual bool isConnected() = 0;

                virtual bool send(const uint8_t *data, size_t length) = 0;
                virtual bool receive(uint8_t *buffer, size_t expected_length, uint32_t timeout_ms) = 0;
                virtual void flush() = 0;
                
                // New methods for frame length calculation
                virtual size_t getHeaderSize() const = 0;
                virtual size_t getFooterSize() const = 0;
                virtual size_t calculateFrameLength(size_t pduLength) const = 0;
                virtual size_t getExceptionResponseLength() const = 0;

                // Get the default packager for this transport
                virtual std::shared_ptr<ModbusPackager> getDefaultPackager() const = 0;

                static constexpr size_t MAX_FRAME_SIZE = 256;
            };

        } // namespace modbus
    } // namespace net
} // namespace speed
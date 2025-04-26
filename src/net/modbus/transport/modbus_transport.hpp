#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace speed {
namespace net {
namespace modbus {

class ModbusTransport {
public:
    virtual ~ModbusTransport() = default;

    virtual bool begin() = 0;
    virtual void stop() = 0;
    virtual bool isConnected() = 0;
    
    virtual bool send(const uint8_t* data, size_t length) = 0;
    virtual bool receive(uint8_t* buffer, size_t expected_length, uint32_t timeout_ms) = 0;
    virtual void flush() = 0;

    static constexpr size_t MAX_FRAME_SIZE = 256;
};

} // namespace modbus
} // namespace net
} // namespace speed
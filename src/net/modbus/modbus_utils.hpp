#pragma once

#include <cstdint>
#include <cstddef>

namespace speed
{
    namespace net
    {
        namespace modbus
        {
            namespace utils
            {
                /**
                 * @brief Calculate Modbus CRC16 (polynomial 0xA001)
                 * 
                 * @param data Pointer to the data buffer
                 * @param length Length of the data buffer
                 * @return uint16_t Calculated CRC
                 */
                uint16_t calculateCRC(const uint8_t *data, size_t length);
            
            } // namespace utils
        } // namespace modbus
    } // namespace net
} // namespace speed
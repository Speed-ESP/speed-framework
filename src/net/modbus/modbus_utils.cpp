#include "modbus_utils.hpp"

namespace speed
{
    namespace net
    {
        namespace modbus
        {
            namespace utils
            {
                uint16_t calculateCRC(const uint8_t *data, size_t length)
                {
                    uint16_t crc = 0xFFFF; // Initialize with 0xFFFF as per Modbus specification
                    
                    for (size_t i = 0; i < length; i++) {
                        crc ^= static_cast<uint16_t>(data[i]); // XOR byte with current CRC
                        
                        // Process each bit in the byte
                        for (int j = 0; j < 8; j++) {
                            // If LSB is 1, shift right and XOR with polynomial
                            if (crc & 0x0001) {
                                crc >>= 1;
                                crc ^= 0xA001; // Modbus polynomial 0xA001 (reverse of 0x8005)
                            } else {
                                // If LSB is 0, just shift right
                                crc >>= 1;
                            }
                        }
                    }
                    
                    return crc;
                }
                
            } // namespace utils
        } // namespace modbus
    } // namespace net
} // namespace speed
#include <net/modbus/packager/modbus_rtu_packager.hpp>
#include <esp_log.h>
#include <inttypes.h>

namespace speed
{
    namespace net
    {
        namespace modbus
        {
            static const char *TAG = "ModbusRtuPackager";

            bool ModbusRtuPackager::packageRequest(const ModbusTransaction& transaction, std::vector<uint8_t>& requestData)
            {
                // Build RTU request: SlaveAddr + FunctionCode + Data + CRC
                requestData.clear();
                requestData.push_back(transaction.slaveAddr);
                requestData.push_back(static_cast<uint8_t>(transaction.function));
                
                // Add start address (register address) - big endian (high byte first)
                requestData.push_back(transaction.startAddress >> 8);
                requestData.push_back(transaction.startAddress & 0xFF);
                
                // Add quantity (number of registers/coils) - big endian
                requestData.push_back(transaction.quantity >> 8);
                requestData.push_back(transaction.quantity & 0xFF);
                
                // Add additional data for write operations
                requestData.insert(requestData.end(), transaction.data.begin(), transaction.data.end());
                
                // Calculate and append CRC
                uint16_t crc = calculateCRC(requestData.data(), requestData.size());
                requestData.push_back(crc & 0xFF);        // Low byte first
                requestData.push_back((crc >> 8) & 0xFF); // High byte second
                
                return true;
            }

            bool ModbusRtuPackager::parseResponse(const std::vector<uint8_t>& responseData, ModbusTransaction& transaction)
            {
                if (responseData.size() < 4) {  // Need at least slave address, function code, and 2-byte CRC
                    ESP_LOGE(TAG, "Response too short");
                    return false;
                }
                
                // Verify CRC
                size_t dataLength = responseData.size() - 2; // Exclude CRC bytes
                uint16_t receivedCrc = (responseData[responseData.size() - 1] << 8) | responseData[responseData.size() - 2];
                uint16_t calculatedCrc = calculateCRC(responseData.data(), dataLength);
                
                if (receivedCrc != calculatedCrc) {
                    ESP_LOGE(TAG, "CRC mismatch: received 0x%04X, calculated 0x%04X", receivedCrc, calculatedCrc);
                    return false;
                }
                
                // Verify slave address matches
                if (responseData[0] != transaction.slaveAddr) {
                    ESP_LOGE(TAG, "Slave address mismatch: expected %" PRIu8 ", got %" PRIu8, 
                             transaction.slaveAddr, responseData[0]);
                    return false;
                }
                
                // Check for exception response
                if (isExceptionResponse(responseData)) {
                    uint8_t exceptionCode = getExceptionCode(responseData);
                    ESP_LOGW(TAG, "Exception response received: 0x%02X", exceptionCode);
                    
                    // Store exception code in transaction data
                    transaction.data.clear();
                    transaction.data.push_back(exceptionCode);
                    transaction.status = TransactionStatus::ExceptionReceived;
                    return true;
                }
                
                // Extract PDU (everything except slave address, CRC)
                transaction.data.clear();
                transaction.data.insert(transaction.data.end(), 
                                      responseData.begin() + 1, // Skip slave address
                                      responseData.end() - 2);  // Skip CRC
                
                transaction.status = TransactionStatus::Success;
                return true;
            }

            size_t ModbusRtuPackager::calculateExpectedResponseLength(const ModbusTransaction& transaction) const
            {
                // Check for exception response
                if (transaction.data.size() >= 2 && (transaction.data[1] & 0x80)) {
                    return getExceptionResponseLength();
                }
                
                // Calculate PDU length based on function code (transport-agnostic)
                size_t pduLength = 1; // Function code
                switch (transaction.function) {
                    case ModbusFunction::ReadCoils:
                    case ModbusFunction::ReadDiscreteInputs:
                        // Byte count + data bytes (8 coils per byte, rounded up)
                        pduLength += 1 + ((transaction.quantity + 7) / 8);
                        break;
                        
                    case ModbusFunction::ReadHoldingRegisters:
                    case ModbusFunction::ReadInputRegisters:
                        // Byte count + data bytes (2 bytes per register)
                        pduLength += 1 + (transaction.quantity * 2);
                        break;
                        
                    case ModbusFunction::WriteSingleCoil:
                    case ModbusFunction::WriteSingleRegister:
                        // Address + value
                        pduLength += 4;
                        break;
                        
                    case ModbusFunction::WriteMultipleCoils:
                    case ModbusFunction::WriteMultipleRegisters:
                        // Start address + quantity
                        pduLength += 4;
                        break;
                        
                    default:
                        // Unknown function code, use minimum response size
                        pduLength += 1;
                        break;
                }
                
                // Add RTU frame components (slave address + CRC)
                return getHeaderSize() + pduLength + getFooterSize();
            }

            size_t ModbusRtuPackager::getExceptionResponseLength() const
            {
                return ModbusConstants::RTU_EXCEPTION_LENGTH;
            }

            bool ModbusRtuPackager::isExceptionResponse(const std::vector<uint8_t>& responseData) const
            {
                // Exception response has function code with high bit set
                return responseData.size() >= 2 && (responseData[1] & 0x80);
            }

            uint8_t ModbusRtuPackager::getExceptionCode(const std::vector<uint8_t>& responseData) const
            {
                return responseData.size() >= 3 ? responseData[2] : 0;
            }

            size_t ModbusRtuPackager::getHeaderSize() const
            {
                return ModbusConstants::RTU_HEADER_SIZE; // Slave address (1 byte)
            }

            size_t ModbusRtuPackager::getFooterSize() const
            {
                return ModbusConstants::RTU_CRC_SIZE; // CRC (2 bytes)
            }

            size_t ModbusRtuPackager::calculateFrameLength(size_t pduLength) const
            {
                return getHeaderSize() + pduLength + getFooterSize();
            }

            uint16_t ModbusRtuPackager::calculateCRC(const uint8_t* data, size_t length) const
            {
                // Use the shared CRC calculation utility
                return utils::calculateCRC(data, length);
            }

        } // namespace modbus
    } // namespace net
} // namespace speed
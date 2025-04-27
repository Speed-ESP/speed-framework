#include <net/modbus/packager/modbus_mbap_packager.hpp>
#include <esp_log.h>
#include <inttypes.h>

namespace speed
{
    namespace net
    {
        namespace modbus
        {
            static const char *TAG = "ModbusMbapPackager";

            ModbusMbapPackager::ModbusMbapPackager()
            {
                // Initialize with a random transaction ID to avoid conflicts
                _nextTransactionId = (esp_random() % 0xFFFF);
            }

            bool ModbusMbapPackager::packageRequest(const ModbusTransaction& transaction, std::vector<uint8_t>& requestData)
            {
                // Build MBAP header and Modbus PDU
                requestData.clear();

                // MBAP Header (7 bytes)
                uint16_t transactionId = transaction.transactionId;
                uint16_t protocolId = ModbusConstants::MODBUS_PROTOCOL_ID; // Always 0 for Modbus
                
                // Calculate PDU length: function code (1) + data
                uint16_t pduLength = 1 + 4 + transaction.data.size(); // Function (1) + Address (2) + Quantity (2) + Data
                
                // Add transaction ID (2 bytes, big endian)
                requestData.push_back((transactionId >> 8) & 0xFF);
                requestData.push_back(transactionId & 0xFF);
                
                // Add protocol ID (2 bytes, big endian, always 0)
                requestData.push_back((protocolId >> 8) & 0xFF);
                requestData.push_back(protocolId & 0xFF);
                
                // Add length (2 bytes, big endian) - unit ID (1) + PDU length
                uint16_t length = 1 + pduLength;
                requestData.push_back((length >> 8) & 0xFF);
                requestData.push_back(length & 0xFF);
                
                // Add unit ID (1 byte, same as slave address)
                requestData.push_back(transaction.slaveAddr);
                
                // Add function code
                requestData.push_back(static_cast<uint8_t>(transaction.function));
                
                // Add start address (2 bytes, big endian)
                requestData.push_back((transaction.startAddress >> 8) & 0xFF);
                requestData.push_back(transaction.startAddress & 0xFF);
                
                // Add quantity (2 bytes, big endian)
                requestData.push_back((transaction.quantity >> 8) & 0xFF);
                requestData.push_back(transaction.quantity & 0xFF);
                
                // Add additional data for write operations
                requestData.insert(requestData.end(), transaction.data.begin(), transaction.data.end());
                
                return true;
            }

            bool ModbusMbapPackager::parseResponse(const std::vector<uint8_t>& responseData, ModbusTransaction& transaction)
            {
                if (responseData.size() < ModbusConstants::TCP_HEADER_SIZE) {
                    ESP_LOGE(TAG, "Response too short for MBAP header");
                    return false;
                }
                
                // Extract MBAP header
                uint16_t transactionId = (responseData[0] << 8) | responseData[1];
                uint16_t protocolId = (responseData[2] << 8) | responseData[3];
                uint16_t length = (responseData[4] << 8) | responseData[5];
                uint8_t unitId = responseData[6];
                
                // Verify transaction ID matches
                if (transactionId != transaction.transactionId) {
                    ESP_LOGE(TAG, "Transaction ID mismatch: expected %u, got %u", 
                             transaction.transactionId, transactionId);
                    return false;
                }
                
                // Verify protocol ID (should be 0)
                if (protocolId != ModbusConstants::MODBUS_PROTOCOL_ID) {
                    ESP_LOGE(TAG, "Invalid Modbus protocol ID: expected 0, got %u", protocolId);
                    return false;
                }
                
                // Verify unit ID matches the slave address
                if (unitId != transaction.slaveAddr) {
                    ESP_LOGE(TAG, "Unit ID mismatch: expected %" PRIu8 ", got %" PRIu8, 
                             transaction.slaveAddr, unitId);
                    return false;
                }
                
                // Verify message length is valid
                if (responseData.size() != length + 6) { // 6 bytes for transaction ID, protocol ID, length
                    ESP_LOGE(TAG, "Length mismatch: header indicates %u bytes, got %zu bytes",
                             length + 6, responseData.size());
                    return false;
                }
                
                // Check for exception response
                if (isExceptionResponse(responseData)) {
                    uint8_t exceptionCode = getExceptionCode(responseData);
                    ESP_LOGW(TAG, "Exception response received: 0x%02X", exceptionCode);
                    
                    transaction.data.clear();
                    transaction.data.push_back(exceptionCode);
                    transaction.status = TransactionStatus::ExceptionReceived;
                    return true;
                }
                
                // Extract PDU (everything except MBAP header)
                transaction.data.clear();
                transaction.data.insert(transaction.data.end(), 
                                        responseData.begin() + ModbusConstants::TCP_HEADER_SIZE, 
                                        responseData.end());
                
                transaction.status = TransactionStatus::Success;
                return true;
            }

            size_t ModbusMbapPackager::calculateExpectedResponseLength(const ModbusTransaction& transaction) const
            {
                // Check for exception response first
                if (transaction.data.size() >= 2 && (transaction.data[1] & 0x80)) {
                    return getExceptionResponseLength();
                }
                
                // Calculate PDU length based on function code
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
                
                // Add MBAP header size
                return getHeaderSize() + pduLength;
            }

            size_t ModbusMbapPackager::getExceptionResponseLength() const
            {
                return ModbusConstants::TCP_EXCEPTION_LENGTH;
            }

            bool ModbusMbapPackager::isExceptionResponse(const std::vector<uint8_t>& responseData) const
            {
                // Exception response has function code with high bit set
                // In MBAP format, function code is at index 7 (after header)
                return responseData.size() >= 8 && (responseData[7] & 0x80);
            }

            uint8_t ModbusMbapPackager::getExceptionCode(const std::vector<uint8_t>& responseData) const
            {
                // In MBAP format, exception code is at index 8 (after header and function code)
                return responseData.size() >= 9 ? responseData[8] : 0;
            }

            size_t ModbusMbapPackager::getHeaderSize() const
            {
                return ModbusConstants::TCP_HEADER_SIZE; // 7 bytes MBAP header
            }

            size_t ModbusMbapPackager::getFooterSize() const
            {
                return 0; // No footer in MBAP
            }

            size_t ModbusMbapPackager::calculateFrameLength(size_t pduLength) const
            {
                return getHeaderSize() + pduLength;
            }

            uint16_t ModbusMbapPackager::getNextTransactionId()
            {
                return _nextTransactionId++;
            }

        } // namespace modbus
    } // namespace net
} // namespace speed
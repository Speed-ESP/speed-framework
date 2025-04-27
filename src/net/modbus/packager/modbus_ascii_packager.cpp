#include <net/modbus/packager/modbus_ascii_packager.hpp>
#include <esp_log.h>
#include <inttypes.h>

namespace speed
{
    namespace net
    {
        namespace modbus
        {
            static const char *TAG = "ModbusAsciiPackager";

            bool ModbusAsciiPackager::packageRequest(const ModbusTransaction& transaction, std::vector<uint8_t>& requestData)
            {
                // Build the binary request first (same as RTU but without CRC)
                std::vector<uint8_t> binaryRequest;
                
                // Add slave address
                binaryRequest.push_back(transaction.slaveAddr);
                
                // Add function code
                binaryRequest.push_back(static_cast<uint8_t>(transaction.function));
                
                // Add start address (register address) - big endian (high byte first)
                binaryRequest.push_back(transaction.startAddress >> 8);
                binaryRequest.push_back(transaction.startAddress & 0xFF);
                
                // Add quantity (number of registers/coils) - big endian
                binaryRequest.push_back(transaction.quantity >> 8);
                binaryRequest.push_back(transaction.quantity & 0xFF);
                
                // Add additional data for write operations
                binaryRequest.insert(binaryRequest.end(), transaction.data.begin(), transaction.data.end());
                
                // Calculate LRC (Longitudinal Redundancy Check)
                uint8_t lrc = calculateLRC(binaryRequest.data(), binaryRequest.size());
                binaryRequest.push_back(lrc);
                
                // Convert binary to ASCII
                std::vector<uint8_t> asciiData = bytesToHex(binaryRequest);
                
                // Prepare the final frame with delimiters
                requestData.clear();
                requestData.push_back(ASCII_START); // Start delimiter ':'
                requestData.insert(requestData.end(), asciiData.begin(), asciiData.end());
                requestData.push_back(ASCII_END_CR); // End delimiter CR
                requestData.push_back(ASCII_END_LF); // End delimiter LF
                
                return true;
            }

            bool ModbusAsciiPackager::parseResponse(const std::vector<uint8_t>& responseData, ModbusTransaction& transaction)
            {
                // Validate minimum length
                if (responseData.size() < 9) { // : + at least 2 chars (1 byte) for address + 2 for function + 2 for LRC + CR + LF
                    ESP_LOGE(TAG, "Response too short: %zu bytes", responseData.size());
                    return false;
                }
                
                // Verify start and end delimiters
                if (responseData[0] != ASCII_START || responseData[responseData.size() - 2] != ASCII_END_CR || 
                    responseData[responseData.size() - 1] != ASCII_END_LF) {
                    ESP_LOGE(TAG, "Invalid frame delimiters");
                    return false;
                }
                
                // Extract ASCII hex content (without delimiters)
                std::vector<uint8_t> asciiContent(responseData.begin() + 1, responseData.end() - 2);
                
                // Convert ASCII hex to binary
                std::vector<uint8_t> binaryContent = hexToBytes(asciiContent);
                
                // Validate minimum binary content length
                if (binaryContent.size() < 3) { // address + function + LRC
                    ESP_LOGE(TAG, "Binary content too short: %zu bytes", binaryContent.size());
                    return false;
                }
                
                // Verify LRC
                size_t contentLength = binaryContent.size() - 1; // Exclude LRC byte
                uint8_t receivedLrc = binaryContent[contentLength];
                uint8_t calculatedLrc = calculateLRC(binaryContent.data(), contentLength);
                
                if (receivedLrc != calculatedLrc) {
                    ESP_LOGE(TAG, "LRC mismatch: received 0x%02X, calculated 0x%02X", receivedLrc, calculatedLrc);
                    return false;
                }
                
                // Verify slave address matches
                if (binaryContent[0] != transaction.slaveAddr) {
                    ESP_LOGE(TAG, "Slave address mismatch: expected %" PRIu8 ", got %" PRIu8, 
                             transaction.slaveAddr, binaryContent[0]);
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
                
                // Extract PDU (everything except slave address and LRC)
                transaction.data.clear();
                transaction.data.insert(transaction.data.end(), 
                                      binaryContent.begin() + 1, // Skip slave address
                                      binaryContent.end() - 1);  // Skip LRC
                
                transaction.status = TransactionStatus::Success;
                return true;
            }

            size_t ModbusAsciiPackager::calculateExpectedResponseLength(const ModbusTransaction& transaction) const
            {
                // For ASCII, each byte becomes 2 ASCII characters.
                // Start with expected PDU length like RTU
                size_t pduLength = 1; // Function code
                
                // Check for exception response
                if (transaction.data.size() >= 2 && (transaction.data[1] & 0x80)) {
                    return getExceptionResponseLength();
                }
                
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
                
                // Calculate ASCII frame length
                // 1 byte slave addr + PDU + 1 byte LRC, converted to ASCII (x2) + delimiters
                size_t binaryLength = 1 + pduLength + 1;
                size_t asciiLength = (binaryLength * 2) + 3; // *2 for hex chars, +3 for :, CR, LF
                
                return asciiLength;
            }

            size_t ModbusAsciiPackager::getExceptionResponseLength() const
            {
                // For exception responses, we have:
                // Slave addr (1 byte) + Function code with MSB set (1 byte) + Exception code (1 byte) + LRC (1 byte)
                // Each byte becomes 2 ASCII chars, plus delimiters
                return (4 * 2) + 3; // 4 bytes * 2 chars/byte + 3 delimiters
            }

            bool ModbusAsciiPackager::isExceptionResponse(const std::vector<uint8_t>& responseData) const
            {
                // For ASCII, we need to convert to binary first
                if (responseData.size() < 9) { // Minimum length for exception response
                    return false;
                }
                
                // Extract ASCII hex content (without delimiters)
                std::vector<uint8_t> asciiContent(responseData.begin() + 1, responseData.end() - 2);
                
                // Convert ASCII hex to binary
                std::vector<uint8_t> binaryContent = hexToBytes(asciiContent);
                
                // Function code with MSB set indicates exception
                return binaryContent.size() >= 2 && (binaryContent[1] & 0x80);
            }

            uint8_t ModbusAsciiPackager::getExceptionCode(const std::vector<uint8_t>& responseData) const
            {
                // Extract ASCII hex content (without delimiters)
                std::vector<uint8_t> asciiContent(responseData.begin() + 1, responseData.end() - 2);
                
                // Convert ASCII hex to binary
                std::vector<uint8_t> binaryContent = hexToBytes(asciiContent);
                
                // Exception code follows function code
                return binaryContent.size() >= 3 ? binaryContent[2] : 0;
            }

            size_t ModbusAsciiPackager::getHeaderSize() const
            {
                return 1; // Start delimiter ':'
            }

            size_t ModbusAsciiPackager::getFooterSize() const
            {
                return 2; // End delimiters CR + LF
            }

            size_t ModbusAsciiPackager::calculateFrameLength(size_t pduLength) const
            {
                // For ASCII, each byte becomes 2 ASCII characters
                // 1 byte slave addr + PDU + 1 byte LRC, converted to ASCII (x2) + delimiters
                size_t binaryLength = 1 + pduLength + 1;
                size_t asciiLength = (binaryLength * 2) + 3; // *2 for hex chars, +3 for :, CR, LF
                
                return asciiLength;
            }

            uint8_t ModbusAsciiPackager::calculateLRC(const uint8_t* data, size_t length) const
            {
                uint8_t lrc = 0;
                
                // LRC is calculated as the 2's complement of the sum of all bytes
                for (size_t i = 0; i < length; i++) {
                    lrc += data[i];
                }
                
                return (~lrc) + 1; // 2's complement
            }

            std::vector<uint8_t> ModbusAsciiPackager::hexToBytes(const std::vector<uint8_t>& hex) const
            {
                std::vector<uint8_t> result;
                
                // Ensure even number of characters (each byte is represented by 2 hex chars)
                if (hex.size() % 2 != 0) {
                    ESP_LOGE(TAG, "Invalid hex string length: %zu (must be even)", hex.size());
                    return result;
                }
                
                // Convert each pair of hex chars to a byte
                for (size_t i = 0; i < hex.size(); i += 2) {
                    uint8_t byte = hexPairToByte(hex[i], hex[i + 1]);
                    result.push_back(byte);
                }
                
                return result;
            }

            std::vector<uint8_t> ModbusAsciiPackager::bytesToHex(const std::vector<uint8_t>& bytes) const
            {
                std::vector<uint8_t> result;
                result.reserve(bytes.size() * 2);
                
                // Convert each byte to a pair of hex chars
                for (uint8_t byte : bytes) {
                    uint8_t high, low;
                    byteToHexPair(byte, high, low);
                    result.push_back(high);
                    result.push_back(low);
                }
                
                return result;
            }

            uint8_t ModbusAsciiPackager::hexPairToByte(uint8_t high, uint8_t low) const
            {
                uint8_t result = 0;
                
                // Convert high nibble
                if (high >= '0' && high <= '9') {
                    result = (high - '0') << 4;
                } else if (high >= 'A' && high <= 'F') {
                    result = (high - 'A' + 10) << 4;
                } else if (high >= 'a' && high <= 'f') {
                    result = (high - 'a' + 10) << 4;
                } else {
                    ESP_LOGE(TAG, "Invalid hex character: 0x%02X", high);
                }
                
                // Convert low nibble
                if (low >= '0' && low <= '9') {
                    result |= (low - '0');
                } else if (low >= 'A' && low <= 'F') {
                    result |= (low - 'A' + 10);
                } else if (low >= 'a' && low <= 'f') {
                    result |= (low - 'a' + 10);
                } else {
                    ESP_LOGE(TAG, "Invalid hex character: 0x%02X", low);
                }
                
                return result;
            }

            void ModbusAsciiPackager::byteToHexPair(uint8_t byte, uint8_t& high, uint8_t& low) const
            {
                // Convert high nibble to hex char
                uint8_t highNibble = (byte >> 4) & 0x0F;
                high = (highNibble <= 9) ? ('0' + highNibble) : ('A' + highNibble - 10);
                
                // Convert low nibble to hex char
                uint8_t lowNibble = byte & 0x0F;
                low = (lowNibble <= 9) ? ('0' + lowNibble) : ('A' + lowNibble - 10);
            }

        } // namespace modbus
    } // namespace net
} // namespace speed
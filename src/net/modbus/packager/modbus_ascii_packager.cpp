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

            void ModbusAsciiPackager::dumpData(const char* label, const std::vector<uint8_t>& data, bool isRequest) const
            {
                if (data.empty()) {
                    ESP_LOGI(TAG, "%s: [empty]", label);
                    return;
                }

                // Ensure we have enough data for a valid ASCII frame
                if (data.size() < 9) { // Minimum: start + 2 chars addr + 2 chars func + 2 chars LRC + CR + LF
                    ESP_LOGI(TAG, "%s: [invalid length: %zu bytes]", label, data.size());
                    return;
                }

                // Verify start and end delimiters
                if (data[0] != ASCII_START || data[data.size() - 2] != ASCII_END_CR || 
                    data[data.size() - 1] != ASCII_END_LF) {
                    ESP_LOGI(TAG, "%s: [invalid delimiters]", label);
                    return;
                }
                
                // Extract ASCII hex content (without delimiters)
                std::vector<uint8_t> asciiContent(data.begin() + 1, data.end() - 2);
                
                // Convert to binary for easier reading
                std::vector<uint8_t> binaryContent = hexToBytes(asciiContent);
                
                if (binaryContent.empty()) {
                    ESP_LOGI(TAG, "%s: [invalid hex encoding]", label);
                    return;
                }
                
                uint8_t slaveAddr = binaryContent[0];
                uint8_t functionCode = binaryContent[1];
                uint8_t lrc = binaryContent[binaryContent.size() - 1];
                
                // Calculate header text width based on type
                const char* typeStr = isRequest ? "REQUEST" : "RESPONSE";
                
                // Print header
                ESP_LOGI(TAG, "┌───────────────────────────────────────────────────────────────┐");
                ESP_LOGI(TAG, "│ MODBUS ASCII %-10s: %-32s │", typeStr, label);
                ESP_LOGI(TAG, "├─────────┬─────────┬─────────────────────────────┬─────────┤");
                ESP_LOGI(TAG, "│ Address │ Function│ Data (Binary)               │ LRC     │");
                ESP_LOGI(TAG, "├─────────┼─────────┼─────────────────────────────┼─────────┤");
                
                // Format fields
                char addrStr[16], funcStr[16], lrcStr[16];
                snprintf(addrStr, sizeof(addrStr), "0x%02X", slaveAddr);
                
                // For function codes, show hex
                if (functionCode & 0x80) {
                    snprintf(funcStr, sizeof(funcStr), "0x%02X", functionCode & 0x7F);
                } else {
                    snprintf(funcStr, sizeof(funcStr), "0x%02X", functionCode);
                }
                
                snprintf(lrcStr, sizeof(lrcStr), "0x%02X", lrc);
                
                // Format data bytes - show binary content
                char dataStr[64] = {0};
                int offset = 0;
                
                // Binary data excluding slave addr, function code and LRC
                for (size_t i = 2; i < binaryContent.size() - 1 && offset < sizeof(dataStr) - 5; i++) {
                    offset += snprintf(dataStr + offset, sizeof(dataStr) - offset, "%02X ", binaryContent[i]);
                    
                    // Add ellipsis if too long
                    if (i >= 10 && i < binaryContent.size() - 2) {
                        snprintf(dataStr + offset, sizeof(dataStr) - offset, "...");
                        break;
                    }
                }
                
                // Print the data row
                ESP_LOGI(TAG, "│ %-7s │ %-7s │ %-27s │ %-7s │", addrStr, funcStr, dataStr, lrcStr);
                
                // Print ASCII representation (original format)
                char asciiStr[64] = {0};
                offset = 0;
                
                // Original ASCII data excluding delimiters
                for (size_t i = 0; i < asciiContent.size() && offset < sizeof(asciiStr) - 5; i++) {
                    offset += snprintf(asciiStr + offset, sizeof(asciiStr) - offset, "%c", asciiContent[i]);
                    
                    // Add ellipsis if too long
                    if (i >= 20 && i < asciiContent.size() - 1) {
                        snprintf(asciiStr + offset, sizeof(asciiStr) - offset, "...");
                        break;
                    }
                }
                
                ESP_LOGI(TAG, "│         │         │ ASCII: %-21s │         │", asciiStr);
                
                // Function code description
                const char* funcDesc = "Unknown";
                if (functionCode & 0x80) {
                    funcDesc = "Exception";
                } else {
                    switch (functionCode) {
                        case 0x01: funcDesc = "Read Coils"; break;
                        case 0x02: funcDesc = "Read Inputs"; break;
                        case 0x03: funcDesc = "Read Holding"; break;
                        case 0x04: funcDesc = "Read Input Reg"; break;
                        case 0x05: funcDesc = "Write Coil"; break;
                        case 0x06: funcDesc = "Write Register"; break;
                        case 0x0F: funcDesc = "Write Coils"; break;
                        case 0x10: funcDesc = "Write Registers"; break;
                        default: break;
                    }
                }
                
                // Print function description
                ESP_LOGI(TAG, "│         │ %-7s │                             │         │", funcDesc);
                
                // Print footer
                ESP_LOGI(TAG, "└─────────┴─────────┴─────────────────────────────┴─────────┘");
                
                // Display raw data in debug level
                if (esp_log_level_get(TAG) >= ESP_LOG_DEBUG) {
                    ESP_LOGD(TAG, "Raw ASCII frame: :%s", std::string(data.begin() + 1, data.end() - 2).c_str());
                    
                    ESP_LOGD(TAG, "Full hex dump of binary data:");
                    for (size_t i = 0; i < binaryContent.size(); i += 16) {
                        char hexLine[50] = {0};
                        char asciiLine[18] = {0};
                        int hexOffset = 0;
                        int asciiOffset = 0;
                        
                        for (size_t j = 0; j < 16 && i + j < binaryContent.size(); j++) {
                            hexOffset += snprintf(hexLine + hexOffset, sizeof(hexLine) - hexOffset, 
                                                 "%02X ", binaryContent[i + j]);
                                                 
                            // Add ASCII representation
                            if (binaryContent[i + j] >= 32 && binaryContent[i + j] <= 126) {
                                asciiOffset += snprintf(asciiLine + asciiOffset, sizeof(asciiLine) - asciiOffset,
                                                      "%c", binaryContent[i + j]);
                            } else {
                                asciiOffset += snprintf(asciiLine + asciiOffset, sizeof(asciiLine) - asciiOffset, ".");
                            }
                        }
                        
                        ESP_LOGD(TAG, "%04zX: %-48s  %s", i, hexLine, asciiLine);
                    }
                }
            }

        } // namespace modbus
    } // namespace net
} // namespace speed
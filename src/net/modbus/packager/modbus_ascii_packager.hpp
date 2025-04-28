#pragma once

#include <net/modbus/packager/modbus_packager.hpp>

namespace speed
{
    namespace net
    {
        namespace modbus
        {
            /**
             * @brief Modbus ASCII packager implementation
             * 
             * Handles ASCII-formatted Modbus frames with start/end delimiters and LRC.
             */
            class ModbusAsciiPackager : public ModbusPackager
            {
            public:
                ModbusAsciiPackager() = default;
                ~ModbusAsciiPackager() override = default;

                // Implement ModbusPackager interface
                bool packageRequest(const ModbusTransaction& transaction, std::vector<uint8_t>& requestData) override;
                bool parseResponse(const std::vector<uint8_t>& responseData, ModbusTransaction& transaction) override;
                size_t calculateExpectedResponseLength(const ModbusTransaction& transaction) const override;
                size_t getExceptionResponseLength() const override;
                bool isExceptionResponse(const std::vector<uint8_t>& responseData) const override;
                uint8_t getExceptionCode(const std::vector<uint8_t>& responseData) const override;
                size_t getHeaderSize() const override;
                size_t getFooterSize() const override;
                size_t calculateFrameLength(size_t pduLength) const override;
                void dumpData(const char* label, const std::vector<uint8_t>& data, bool isRequest) const override;
            private:
                // ASCII mode constants
                static constexpr char ASCII_START = ':';   // Start of frame
                static constexpr char ASCII_END_CR = '\r'; // End of frame CR
                static constexpr char ASCII_END_LF = '\n'; // End of frame LF
                
                uint8_t calculateLRC(const uint8_t* data, size_t length) const;
                std::vector<uint8_t> hexToBytes(const std::vector<uint8_t>& hex) const;
                std::vector<uint8_t> bytesToHex(const std::vector<uint8_t>& bytes) const;
                uint8_t hexPairToByte(uint8_t high, uint8_t low) const;
                void byteToHexPair(uint8_t byte, uint8_t& high, uint8_t& low) const;
            };

        } // namespace modbus
    } // namespace net
} // namespace speed
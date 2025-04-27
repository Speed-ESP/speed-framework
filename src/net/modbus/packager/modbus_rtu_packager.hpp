#pragma once

#include <net/modbus/packager/modbus_packager.hpp>
#include <net/modbus/modbus_utils.hpp>

namespace speed
{
    namespace net
    {
        namespace modbus
        {
            /**
             * @brief Modbus RTU packager implementation
             * 
             * Handles RTU-formatted Modbus frames with slave address and CRC.
             */
            class ModbusRtuPackager : public ModbusPackager
            {
            public:
                ModbusRtuPackager() = default;
                ~ModbusRtuPackager() override = default;

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

            private:
                uint16_t calculateCRC(const uint8_t* data, size_t length) const;
            };

        } // namespace modbus
    } // namespace net
} // namespace speed
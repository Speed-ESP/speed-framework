#pragma once

#include <atomic>
#include <net/modbus/packager/modbus_packager.hpp>

namespace speed
{
    namespace net
    {
        namespace modbus
        {
            /**
             * @brief MBAP (Modbus TCP) packager implementation
             * 
             * Handles MBAP-formatted Modbus frames with transaction ID, protocol ID,
             * length field, and unit ID.
             */
            class ModbusMbapPackager : public ModbusPackager
            {
            public:
                ModbusMbapPackager();
                ~ModbusMbapPackager() override = default;

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
                std::atomic<uint16_t> _nextTransactionId{0};
                uint16_t getNextTransactionId();
            };

        } // namespace modbus
    } // namespace net
} // namespace speed
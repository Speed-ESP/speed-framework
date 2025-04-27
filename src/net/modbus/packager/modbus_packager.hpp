#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <net/modbus/modbus_defs.hpp>

namespace speed
{
    namespace net
    {
        namespace modbus
        {
            /**
             * @brief Abstract interface for Modbus packager implementations.
             * 
             * ModbusPackager is responsible for handling different Modbus package formats:
             * - MBAP (Modbus TCP)
             * - RTU (Serial with CRC)
             * - ASCII (Serial with LRC)
             * 
             * The packager formats requests and parses responses, but does not handle
             * transport-specific details like socket connections or UART config.
             */
            class ModbusPackager
            {
            public:
                virtual ~ModbusPackager() = default;

                /**
                 * @brief Package a Modbus request into the appropriate format
                 * 
                 * @param transaction The transaction information to package
                 * @param requestData Output buffer for the formatted request
                 * @return true if packaging was successful
                 */
                virtual bool packageRequest(const ModbusTransaction& transaction, std::vector<uint8_t>& requestData) = 0;

                /**
                 * @brief Parse a Modbus response and extract the PDU
                 * 
                 * @param responseData The raw response from the transport
                 * @param transaction The transaction to update with parsed data
                 * @return true if the response was valid and parsed successfully
                 */
                virtual bool parseResponse(const std::vector<uint8_t>& responseData, ModbusTransaction& transaction) = 0;

                /**
                 * @brief Calculate the expected length of a response based on function code
                 * 
                 * @param transaction The transaction to calculate the response length for
                 * @return The expected length of the response in bytes
                 */
                virtual size_t calculateExpectedResponseLength(const ModbusTransaction& transaction) const = 0;

                /**
                 * @brief Get the exception response length
                 * 
                 * @return size_t Length of an exception response in bytes
                 */
                virtual size_t getExceptionResponseLength() const = 0;

                /**
                 * @brief Check if a response is an exception
                 * 
                 * @param responseData The response data to check
                 * @return true if the response is an exception
                 */
                virtual bool isExceptionResponse(const std::vector<uint8_t>& responseData) const = 0;

                /**
                 * @brief Get the exception code from an exception response
                 * 
                 * @param responseData The exception response
                 * @return uint8_t The exception code
                 */
                virtual uint8_t getExceptionCode(const std::vector<uint8_t>& responseData) const = 0;

                /**
                 * @brief Get the header size used by this packager
                 * 
                 * @return size_t Header size in bytes
                 */
                virtual size_t getHeaderSize() const = 0;

                /**
                 * @brief Get the footer size used by this packager
                 * 
                 * @return size_t Footer size in bytes
                 */
                virtual size_t getFooterSize() const = 0;

                /**
                 * @brief Calculate frame length for a given PDU length
                 * 
                 * @param pduLength The length of the PDU
                 * @return size_t Total frame length including header and footer
                 */
                virtual size_t calculateFrameLength(size_t pduLength) const = 0;
            };

        } // namespace modbus
    } // namespace net
} // namespace speed
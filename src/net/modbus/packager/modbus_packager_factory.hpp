#pragma once

#include <memory>
#include <net/modbus/packager/modbus_packager.hpp>
#include <net/modbus/packager/modbus_rtu_packager.hpp>
#include <net/modbus/packager/modbus_mbap_packager.hpp>
#include <net/modbus/packager/modbus_ascii_packager.hpp>

namespace speed
{
    namespace net
    {
        namespace modbus
        {
            /**
             * @brief Factory class for creating Modbus packager instances
             */
            class ModbusPackagerFactory
            {
            public:
                /**
                 * @brief Create an RTU packager
                 * @return std::shared_ptr<ModbusPackager> RTU packager instance
                 */
                static std::shared_ptr<ModbusPackager> createRtuPackager()
                {
                    return std::make_shared<ModbusRtuPackager>();
                }

                /**
                 * @brief Create an MBAP (TCP) packager
                 * @return std::shared_ptr<ModbusPackager> MBAP packager instance
                 */
                static std::shared_ptr<ModbusPackager> createMbapPackager()
                {
                    return std::make_shared<ModbusMbapPackager>();
                }

                /**
                 * @brief Create an ASCII packager
                 * @return std::shared_ptr<ModbusPackager> ASCII packager instance
                 */
                static std::shared_ptr<ModbusPackager> createAsciiPackager()
                {
                    return std::make_shared<ModbusAsciiPackager>();
                }

                /**
                 * @brief Create a default packager for the specified transport type
                 * @param isSerialTransport True if the transport is serial-based (UART), false for network-based (TCP)
                 * @return std::shared_ptr<ModbusPackager> Default packager for the transport
                 */
                static std::shared_ptr<ModbusPackager> createDefaultPackager(bool isSerialTransport)
                {
                    return isSerialTransport ? createRtuPackager() : createMbapPackager();
                }
            };

        } // namespace modbus
    } // namespace net
} // namespace speed
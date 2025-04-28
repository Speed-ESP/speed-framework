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
            };

        } // namespace modbus
    } // namespace net
} // namespace speed
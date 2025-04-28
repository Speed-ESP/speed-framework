#pragma once

#include <memory>

namespace speed
{
    namespace net
    {
        namespace modbus
        {
            // Forward declaration
            class ModbusPackager;

            struct ModbusConfig
            {
                static constexpr int16_t DEFAULT_STACK_SIZE = 4096 * 4; // 16KB
                static constexpr uint32_t DEFAULT_TASK_PRIORITY = 5;
                static constexpr uint32_t DEFAULT_QUEUE_SIZE = 100;
                static constexpr uint32_t DEFAULT_UART_BUFFER_SIZE = 1024;
                static constexpr uint32_t DEFAULT_RESPONSE_TIMEOUT_MS = 1000;
                static constexpr uint32_t DEFAULT_QUEUE_TIMEOUT_MS = 100;
                static constexpr uint32_t DEFAULT_READ_INTERVAL_MS = 10;

                uint32_t stackSize = DEFAULT_STACK_SIZE;
                uint32_t taskPriority = DEFAULT_TASK_PRIORITY;
                uint32_t queueSize = DEFAULT_QUEUE_SIZE;
                uint32_t uartBufferSize = DEFAULT_UART_BUFFER_SIZE;
                uint32_t responseTimeoutMs = DEFAULT_RESPONSE_TIMEOUT_MS;
                uint32_t queueTimeoutMs = DEFAULT_QUEUE_TIMEOUT_MS;
                uint32_t readIntervalMs = DEFAULT_READ_INTERVAL_MS;
                
                // Custom packager to use (if null, a default packager will be created based on transport type)
                std::shared_ptr<ModbusPackager> packager = nullptr;
            };

        } // namespace modbus
    } // namespace net
} // namespace speed
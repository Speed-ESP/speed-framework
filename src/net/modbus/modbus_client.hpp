#pragma once

#include <memory>
#include <map>
#include <string>
#include <functional>

#include <net/modbus/modbus_master.hpp>

namespace speed
{
    namespace net
    {
        namespace modbus
        {

            // Represents a register or coil value with metadata
            struct ModbusValue
            {
                uint16_t address;
                uint16_t value;
                uint32_t timestamp;
                bool valid;
            };

            // Callback for value updates
            using ValueUpdateCallback = std::function<void(const ModbusValue &value)>;
            // Callback for value change monitoring
            using ValueChangeCallback = std::function<void(const ModbusValue &oldValue, const ModbusValue &newValue)>;

            class ModbusClient
            {
                private:
                  struct PollInfo;
            public:
                ModbusClient(std::shared_ptr<ModbusMaster> master, uint8_t slaveAddress, const std::string &name = "");
                ~ModbusClient();

                // Device information
                uint8_t getSlaveAddress() const { return _slaveAddress; }
                const std::string &getName() const { return _name; }
                bool isConnected() const { return _master && _master->isConnected(); }

                // Register operations with caching
                bool readHoldingRegister(uint16_t address, ValueUpdateCallback callback = nullptr);
                bool readMultipleHoldingRegisters(uint16_t startAddress, uint16_t quantity,
                                                  std::function<void(const std::vector<ModbusValue> &)> callback = nullptr);
                bool writeHoldingRegister(uint16_t address, uint16_t value, ValueUpdateCallback callback = nullptr);
                bool writeMultipleHoldingRegisters(uint16_t startAddress, const std::vector<uint16_t> &values,
                                                   std::function<void(bool success)> callback = nullptr);

                // Coil operations
                bool readCoil(uint16_t address, ValueUpdateCallback callback = nullptr);
                bool writeCoil(uint16_t address, bool value, ValueUpdateCallback callback = nullptr);

                // Cache management
                ModbusValue getCachedValue(uint16_t address) const;
                void clearCache();
                void setMaxCacheAge(uint32_t maxAgeMs) { _maxCacheAge = maxAgeMs; }

                // Polling configuration - modified for centralized polling
                void enablePolling(uint16_t address, uint32_t intervalMs = 0, ValueUpdateCallback callback = nullptr);
                void disablePolling(uint16_t address);
                void setPollInterval(uint16_t address, uint32_t intervalMs);
                bool isPollingEnabled(uint16_t address) const;
                const std::map<uint16_t, PollInfo> &getPollConfig() const { return _pollConfig; }
                ModbusValue pollRegister(uint16_t address); // New method for device manager to poll

                // Value change monitoring
                void onValueChange(uint16_t address, ValueChangeCallback callback);
                void removeValueChangeCallback(uint16_t address);

            private:
                // Polling management - simplified for centralized polling
                struct PollInfo
                {
                    uint32_t intervalMs;
                    uint32_t lastPollTime;
                    ValueUpdateCallback callback;
                };

                std::shared_ptr<ModbusMaster> _master;
                uint8_t _slaveAddress;
                std::string _name;

                // Cache management
                std::map<uint16_t, ModbusValue> _cache;
                uint32_t _maxCacheAge{5000}; // Default 5 seconds

                std::map<uint16_t, PollInfo> _pollConfig;
                SemaphoreHandle_t _pollConfigMutex{nullptr};

                // Value change monitoring
                std::map<uint16_t, ValueChangeCallback> _changeCallbacks;

                void updateCache(uint16_t address, uint16_t value);
                bool isCacheValid(const ModbusValue &value) const;
                void handleModbusResponse(uint16_t address, const std::vector<uint8_t> &data,
                                          TransactionStatus status, ValueUpdateCallback callback);
            };

        } // namespace modbus
    } // namespace net
} // namespace speed
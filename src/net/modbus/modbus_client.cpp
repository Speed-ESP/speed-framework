#include <net/modbus/modbus_client.hpp>
#include <net/modbus/modbus_utils.hpp>
#include "esp_log.h"
#include <inttypes.h>
namespace speed
{
    namespace net
    {
        namespace modbus
        {

            static const char *TAG = "ModbusClient";

            ModbusClient::ModbusClient(std::shared_ptr<ModbusMaster> master, uint8_t slaveAddress, const std::string &name)
                : _master(master), _slaveAddress(slaveAddress), _name(name) 
            {
                _pollConfigMutex = xSemaphoreCreateMutex();
                // No longer starting polling task here as polling is now managed by the device manager
            }

            ModbusClient::~ModbusClient()
            {
                if (_pollConfigMutex)
                {
                    vSemaphoreDelete(_pollConfigMutex);
                    _pollConfigMutex = nullptr;
                }
            }

            bool ModbusClient::readHoldingRegister(uint16_t address, ValueUpdateCallback callback)
            {
                // Check cache first
                if (callback)
                {
                    auto cachedValue = getCachedValue(address);
                    if (cachedValue.valid && isCacheValid(cachedValue))
                    {
                        callback(cachedValue);
                        return true;
                    }
                }

                // Create callback wrapper to handle cache update
                auto responseCallback = [this, address, callback](uint8_t slaveAddr, ModbusFunction function,
                                                                  const std::vector<uint8_t> &data, TransactionStatus status)
                {
                    this->handleModbusResponse(address, data, status, callback);
                };

                return _master->readHoldingRegisters(_slaveAddress, address, 1, responseCallback);
            }

            bool ModbusClient::readMultipleHoldingRegisters(uint16_t startAddress, uint16_t quantity,
                                                            std::function<void(const std::vector<ModbusValue> &)> callback)
            {
                auto responseCallback = [this, startAddress, quantity, callback](uint8_t slaveAddr, ModbusFunction function,
                                                                                 const std::vector<uint8_t> &data, TransactionStatus status)
                {
                    if (status == TransactionStatus::Success && data.size() >= 3)
                    {
                        std::vector<ModbusValue> values;
                        values.reserve(quantity);

                        for (uint16_t i = 0; i < quantity; i++)
                        {
                            uint16_t value = (data[3 + i * 2] << 8) | data[4 + i * 2];
                            uint16_t address = startAddress + i;

                            ModbusValue modbusValue{
                                .address = address,
                                .value = value,
                                .timestamp = xTaskGetTickCount(),
                                .valid = true};

                            values.push_back(modbusValue);
                            updateCache(address, value);
                        }

                        if (callback)
                        {
                            callback(values);
                        }
                    }
                    else if (callback)
                    {
                        callback(std::vector<ModbusValue>());
                    }
                };

                return _master->readHoldingRegisters(_slaveAddress, startAddress, quantity, responseCallback);
            }

            bool ModbusClient::writeHoldingRegister(uint16_t address, uint16_t value, ValueUpdateCallback callback)
            {
                auto responseCallback = [this, address, value, callback](uint8_t slaveAddr, ModbusFunction function,
                                                                         const std::vector<uint8_t> &data, TransactionStatus status)
                {
                    if (status == TransactionStatus::Success)
                    {
                        updateCache(address, value);
                        if (callback)
                        {
                            ModbusValue modbusValue{
                                .address = address,
                                .value = value,
                                .timestamp = xTaskGetTickCount(),
                                .valid = true};
                            callback(modbusValue);
                        }
                    }
                    else if (callback)
                    {
                        ModbusValue modbusValue{
                            .address = address,
                            .value = 0,
                            .timestamp = xTaskGetTickCount(),
                            .valid = false};
                        callback(modbusValue);
                    }
                };

                return _master->writeSingleRegister(_slaveAddress, address, value, responseCallback);
            }

            bool ModbusClient::writeMultipleHoldingRegisters(uint16_t startAddress, const std::vector<uint16_t> &values,
                                                             std::function<void(bool success)> callback)
            {
                auto responseCallback = [this, startAddress, values, callback](uint8_t slaveAddr, ModbusFunction function,
                                                                               const std::vector<uint8_t> &data, TransactionStatus status)
                {
                    bool success = (status == TransactionStatus::Success);
                    if (success)
                    {
                        for (size_t i = 0; i < values.size(); i++)
                        {
                            updateCache(startAddress + i, values[i]);
                        }
                    }
                    if (callback)
                    {
                        callback(success);
                    }
                };

                return _master->writeMultipleRegisters(_slaveAddress, startAddress, values, responseCallback);
            }

            bool ModbusClient::readCoil(uint16_t address, ValueUpdateCallback callback)
            {
                auto responseCallback = [this, address, callback](uint8_t slaveAddr, ModbusFunction function,
                                                                  const std::vector<uint8_t> &data, TransactionStatus status)
                {
                    if (status == TransactionStatus::Success && data.size() >= 3)
                    {
                        uint16_t value = (data[3] & 0x01) ? 1 : 0;
                        updateCache(address, value);

                        if (callback)
                        {
                            ModbusValue modbusValue{
                                .address = address,
                                .value = value,
                                .timestamp = xTaskGetTickCount(),
                                .valid = true};
                            callback(modbusValue);
                        }
                    }
                    else if (callback)
                    {
                        ModbusValue modbusValue{
                            .address = address,
                            .value = 0,
                            .timestamp = xTaskGetTickCount(),
                            .valid = false};
                        callback(modbusValue);
                    }
                };

                return _master->readCoils(_slaveAddress, address, 1, responseCallback);
            }

            bool ModbusClient::writeCoil(uint16_t address, bool value, ValueUpdateCallback callback)
            {
                auto responseCallback = [this, address, value, callback](uint8_t slaveAddr, ModbusFunction function,
                                                                         const std::vector<uint8_t> &data, TransactionStatus status)
                {
                    if (status == TransactionStatus::Success)
                    {
                        updateCache(address, value ? static_cast<uint16_t>(1) : static_cast<uint16_t>(0));
                        if (callback)
                        {
                            ModbusValue modbusValue{
                                .address = address,
                                .value = static_cast<uint16_t>(value),
                                .timestamp = xTaskGetTickCount(),
                                .valid = true};
                            callback(modbusValue);
                        }
                    }
                    else if (callback)
                    {
                        ModbusValue modbusValue{
                            .address = address,
                            .value = 0,
                            .timestamp = xTaskGetTickCount(),
                            .valid = false};
                        callback(modbusValue);
                    }
                };

                return _master->writeSingleCoil(_slaveAddress, address, value, responseCallback);
            }

            ModbusValue ModbusClient::getCachedValue(uint16_t address) const
            {
                auto it = _cache.find(address);
                if (it != _cache.end() && isCacheValid(it->second))
                {
                    return it->second;
                }
                return ModbusValue{
                    .address = address,
                    .value = 0,
                    .timestamp = 0,
                    .valid = false};
            }

            void ModbusClient::clearCache()
            {
                _cache.clear();
            }

            void ModbusClient::enablePolling(uint16_t address, uint32_t intervalMs, ValueUpdateCallback callback)
            {
                if (intervalMs < ModbusConstants::MIN_POLL_INTERVAL_MS)
                {
                    ESP_LOGW(TAG, "Poll interval %" PRIu32 " ms is too small, using minimum %" PRIu32 " ms",
                             intervalMs, ModbusConstants::MIN_POLL_INTERVAL_MS);
                    intervalMs = ModbusConstants::MIN_POLL_INTERVAL_MS;
                }

                if (xSemaphoreTake(_pollConfigMutex, pdMS_TO_TICKS(100)) == pdTRUE)
                {
                    PollInfo pollInfo{
                        .intervalMs = intervalMs,
                        .lastPollTime = 0,
                        .callback = callback};

                    _pollConfig[address] = pollInfo;
                    xSemaphoreGive(_pollConfigMutex);
                    ESP_LOGI(TAG, "Enabled polling for address 0x%04X with interval %" PRIu32 " ms", address, intervalMs);
                }
                else
                {
                    ESP_LOGW(TAG, "Failed to enable polling for address 0x%04X: could not obtain mutex", address);
                }
            }

            void ModbusClient::disablePolling(uint16_t address)
            {
                if (xSemaphoreTake(_pollConfigMutex, pdMS_TO_TICKS(100)) == pdTRUE)
                {
                    auto it = _pollConfig.find(address);
                    if (it != _pollConfig.end())
                    {
                        _pollConfig.erase(it);
                        xSemaphoreGive(_pollConfigMutex);
                        ESP_LOGI(TAG, "Disabled polling for address 0x%04X", address);
                    }
                    else
                    {
                        xSemaphoreGive(_pollConfigMutex);
                    }
                }
                else
                {
                    ESP_LOGW(TAG, "Failed to disable polling for address 0x%04X: could not obtain mutex", address);
                }
            }

            void ModbusClient::setPollInterval(uint16_t address, uint32_t intervalMs)
            {
                if (xSemaphoreTake(_pollConfigMutex, pdMS_TO_TICKS(100)) == pdTRUE)
                {
                    auto it = _pollConfig.find(address);
                    if (it != _pollConfig.end())
                    {
                        if (intervalMs < ModbusConstants::MIN_POLL_INTERVAL_MS)
                        {
                            ESP_LOGW(TAG, "Poll interval %" PRIu32 " ms is too small, using minimum %" PRIu32 " ms",
                                     intervalMs, ModbusConstants::MIN_POLL_INTERVAL_MS);
                            intervalMs = ModbusConstants::MIN_POLL_INTERVAL_MS;
                        }
                        it->second.intervalMs = intervalMs;
                        xSemaphoreGive(_pollConfigMutex);
                        ESP_LOGI(TAG, "Updated polling interval for address 0x%04X to %" PRIu32 " ms", address, intervalMs);
                    }
                    else
                    {
                        xSemaphoreGive(_pollConfigMutex);
                        ESP_LOGW(TAG, "Cannot set poll interval: address 0x%04X is not configured for polling", address);
                    }
                }
                else
                {
                    ESP_LOGW(TAG, "Failed to set polling interval for address 0x%04X: could not obtain mutex", address);
                }
            }

            void ModbusClient::onValueChange(uint16_t address, ValueChangeCallback callback)
            {
                _changeCallbacks[address] = callback;
                ESP_LOGI(TAG, "Added value change callback for address 0x%04X", address);
            }

            void ModbusClient::removeValueChangeCallback(uint16_t address)
            {
                auto it = _changeCallbacks.find(address);
                if (it != _changeCallbacks.end())
                {
                    _changeCallbacks.erase(it);
                    ESP_LOGI(TAG, "Removed value change callback for address 0x%04X", address);
                }
            }

            void ModbusClient::updateCache(uint16_t address, uint16_t value)
            {
                // Get old value before updating
                auto oldValue = getCachedValue(address);

                // Create new value
                ModbusValue newValue{
                    .address = address,
                    .value = value,
                    .timestamp = xTaskGetTickCount(),
                    .valid = true};

                // Update cache
                _cache[address] = newValue;

                // Check for value change callback
                auto it = _changeCallbacks.find(address);
                if (it != _changeCallbacks.end() && (!oldValue.valid || oldValue.value != value))
                {
                    it->second(oldValue, newValue);
                }
            }

            bool ModbusClient::isCacheValid(const ModbusValue &value) const
            {
                if (!value.valid)
                    return false;
                uint32_t age = xTaskGetTickCount() - value.timestamp;
                return age <= pdMS_TO_TICKS(_maxCacheAge);
            }

            void ModbusClient::handleModbusResponse(uint16_t address, const std::vector<uint8_t> &data,
                                                    TransactionStatus status, ValueUpdateCallback callback)
            {
                if (status == TransactionStatus::Success && data.size() >= 3)
                {
                    uint16_t value = (data[3] << 8) | data[4];
                    updateCache(address, value);

                    if (callback)
                    {
                        ModbusValue modbusValue{
                            .address = address,
                            .value = value,
                            .timestamp = xTaskGetTickCount(),
                            .valid = true};
                        callback(modbusValue);
                    }
                }
                else if (callback)
                {
                    ModbusValue modbusValue{
                        .address = address,
                        .value = 0,
                        .timestamp = xTaskGetTickCount(),
                        .valid = false};
                    callback(modbusValue);
                }
            }

            ModbusValue ModbusClient::pollRegister(uint16_t address)
            {
                // Update the last poll time
                if (xSemaphoreTake(_pollConfigMutex, pdMS_TO_TICKS(100)) == pdTRUE)
                {
                    auto it = _pollConfig.find(address);
                    if (it != _pollConfig.end())
                    {
                        it->second.lastPollTime = xTaskGetTickCount();
                        
                        // Store the callback locally before releasing the mutex
                        auto callback = it->second.callback;
                        xSemaphoreGive(_pollConfigMutex);
                        
                        // Perform the actual read operation
                        bool success = readHoldingRegister(address, callback);
                        
                        if (success)
                        {
                            // If successful, return the newly cached value
                            return getCachedValue(address);
                        }
                    }
                    else
                    {
                        xSemaphoreGive(_pollConfigMutex);
                    }
                }
                
                // Return invalid value if the poll failed or the address is not configured
                return ModbusValue{
                    .address = address,
                    .value = 0,
                    .timestamp = 0,
                    .valid = false
                };
            }

            bool ModbusClient::isPollingEnabled(uint16_t address) const
            {
                if (xSemaphoreTake(_pollConfigMutex, pdMS_TO_TICKS(100)) == pdTRUE)
                {
                    bool enabled = _pollConfig.find(address) != _pollConfig.end();
                    xSemaphoreGive(_pollConfigMutex);
                    return enabled;
                }
                
                return false; // If we couldn't take the mutex, assume polling is disabled
            }

        } // namespace modbus
    } // namespace net
} // namespace speed
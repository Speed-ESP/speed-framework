#include <net/modbus/modbus_device_manager.hpp>
#include "esp_log.h"
#include <atomic>
#include "freertos/mpu_wrappers.h"

namespace speed
{
    namespace net
    {
        namespace modbus
        {

            static const char *TAG = "ModbusDeviceManager";
            static constexpr uint32_t SCAN_STACK_SIZE = 4096;
            static constexpr uint32_t SCAN_TASK_PRIORITY = 5;
            static constexpr uint32_t SCAN_TIMEOUT_MS = 200;

            // Create scan parameters structure
            struct ScanParams
            {
                ModbusDeviceManager *manager;
                uint8_t startAddr;
                uint8_t endAddr;
                DeviceDiscoveryCallback callback;
            };

            ModbusDeviceManager::ModbusDeviceManager(std::shared_ptr<ModbusMaster> master)
                : _master(master) {}

            std::shared_ptr<ModbusClient> ModbusDeviceManager::addDevice(uint8_t address, const std::string &name)
            {
                if (hasDevice(address))
                {
                    ESP_LOGW(TAG, "Device with address %d already exists", address);
                    return _devices[address];
                }

                auto client = std::make_shared<ModbusClient>(_master, address, name);
                _devices[address] = client;
                ESP_LOGI(TAG, "Added device %s with address %d", name.empty() ? "unnamed" : name.c_str(), address);
                return client;
            }

            void ModbusDeviceManager::removeDevice(uint8_t address)
            {
                auto it = _devices.find(address);
                if (it != _devices.end())
                {
                    ESP_LOGI(TAG, "Removing device %s with address %d",
                             it->second->getName().empty() ? "unnamed" : it->second->getName().c_str(), address);
                    _devices.erase(it);
                }
            }

            std::shared_ptr<ModbusClient> ModbusDeviceManager::getDevice(uint8_t address)
            {
                auto it = _devices.find(address);
                return (it != _devices.end()) ? it->second : nullptr;
            }

            std::vector<std::shared_ptr<ModbusClient>> ModbusDeviceManager::getAllDevices() const
            {
                std::vector<std::shared_ptr<ModbusClient>> devices;
                devices.reserve(_devices.size());
                for (const auto &pair : _devices)
                {
                    devices.push_back(pair.second);
                }
                return devices;
            }

            void ModbusDeviceManager::scanNetwork(uint8_t startAddr, uint8_t endAddr, DeviceDiscoveryCallback callback)
            {
                if (_scanning)
                {
                    ESP_LOGW(TAG, "Network scan already in progress");
                    return;
                }

                if (startAddr > endAddr || endAddr > ModbusConstants::MAX_DEVICE_ADDRESS)
                {
                    ESP_LOGE(TAG, "Invalid address range: %d - %d", startAddr, endAddr);
                    return;
                }

                _scanning = true;

                // Create scan parameters
                auto params = new ScanParams{
                    .manager = this,
                    .startAddr = startAddr,
                    .endAddr = endAddr,
                    .callback = callback};

                // Start scan task
                BaseType_t ret = xTaskCreate(
                    scanTaskFunction,
                    "modbus_scan",
                    SCAN_STACK_SIZE,
                    params,
                    SCAN_TASK_PRIORITY,
                    &_scanTask);

                if (ret != pdPASS)
                {
                    ESP_LOGE(TAG, "Failed to create scan task");
                    delete params;
                    _scanning = false;
                }
            }

            void ModbusDeviceManager::stopScan()
            {
                if (_scanning && _scanTask != nullptr)
                {
                    vTaskDelete(_scanTask);
                    _scanTask = nullptr;
                    _scanning = false;
                    ESP_LOGI(TAG, "Network scan stopped");
                }
            }

            void ModbusDeviceManager::scanTaskFunction(void *param)
            {
                auto params = static_cast<ScanParams *>(param);
                params->manager->processScanTask(params->startAddr, params->endAddr, params->callback);
                delete params;

                params->manager->_scanning = false;
                params->manager->_scanTask = nullptr;
                vTaskDelete(nullptr);
            }

            void ModbusDeviceManager::processScanTask(uint8_t startAddr, uint8_t endAddr, DeviceDiscoveryCallback callback)
            {
                ESP_LOGI(TAG, "Starting network scan from address %d to %d", startAddr, endAddr);

                for (uint8_t addr = startAddr; addr <= endAddr && _scanning; addr++)
                {
                    bool present = false;

                    // Try multiple times for each address
                    for (int attempt = 0; attempt < ModbusConstants::MAX_RETRIES && !present; attempt++)
                    {
                        if (attempt > 0)
                        {
                            // Add delay between retries
                            vTaskDelay(pdMS_TO_TICKS(100));
                        }

                        present = testDevicePresence(addr);
                    }

                    if (callback)
                    {
                        callback(addr, present);
                    }

                    // Add delay between addresses to avoid flooding the network
                    vTaskDelay(pdMS_TO_TICKS(50));
                }

                ESP_LOGI(TAG, "Network scan completed");
            }

            bool ModbusDeviceManager::testDevicePresence(uint8_t address)
            {
                // Try to read a single holding register to test device presence
                std::atomic<bool> responseReceived{false};

                auto callback = [&responseReceived](uint8_t slaveAddr, ModbusFunction function,
                                                    const std::vector<uint8_t> &data, TransactionStatus status)
                {
                    responseReceived = (status == TransactionStatus::Success);
                };

                if (!_master->readHoldingRegisters(address, 0, 1, callback))
                {
                    return false;
                }

                // Wait for response with timeout
                TickType_t startTime = xTaskGetTickCount();
                while (!responseReceived)
                {
                    if ((xTaskGetTickCount() - startTime) > pdMS_TO_TICKS(SCAN_TIMEOUT_MS))
                    {
                        return false;
                    }
                    vTaskDelay(pdMS_TO_TICKS(1));
                }

                return responseReceived;
            }

            const ModbusStatistics &ModbusDeviceManager::getNetworkStatistics() const
            {
                return _master->getStatistics();
            }

            void ModbusDeviceManager::resetNetworkStatistics()
            {
                _master->resetStatistics();
            }

            void ModbusDeviceManager::disconnectAll()
            {
                _devices.clear();
                ESP_LOGI(TAG, "All devices disconnected");
            }

        } // namespace modbus
    } // namespace net
} // namespace speed
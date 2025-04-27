#include "freertos/mpu_wrappers.h"
#include <net/modbus/modbus_device_manager.hpp>
#include "esp_log.h"
#include <atomic>
#include <inttypes.h>

namespace speed
{
    namespace net
    {
        namespace modbus
        {

            static const char *TAG = "ModbusDeviceManager";
            static constexpr uint32_t SCAN_STACK_SIZE = 4096;
            static constexpr uint32_t SCAN_TASK_PRIORITY = 2;
            static constexpr uint32_t SCAN_TIMEOUT_MS = 200;
            static constexpr uint32_t POLLING_TASK_PRIORIT = 2;

            // Create scan parameters structure
            struct ScanParams
            {
                ModbusDeviceManager *manager;
                uint8_t startAddr;
                uint8_t endAddr;
                DeviceDiscoveryCallback callback;
            };

            ModbusDeviceManager::ModbusDeviceManager(std::shared_ptr<ModbusMaster> master)
                : _master(master)
            {
                _devicesMutex = xSemaphoreCreateMutex();
            }

            std::shared_ptr<ModbusClient> ModbusDeviceManager::addDevice(uint8_t address, const std::string &name)
            {
                if (hasDevice(address))
                {
                    ESP_LOGW(TAG, "Device with address %" PRIu8 " already exists", address);
                    return _devices[address];
                }

                auto client = std::make_shared<ModbusClient>(_master, address, name);
                _devices[address] = client;
                ESP_LOGI(TAG, "Added device %s with address %" PRIu8, name.empty() ? "unnamed" : name.c_str(), address);
                return client;
            }

            void ModbusDeviceManager::removeDevice(uint8_t address)
            {
                auto it = _devices.find(address);
                if (it != _devices.end())
                {
                    ESP_LOGI(TAG, "Removing device %s with address %" PRIu8,
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
                    ESP_LOGE(TAG, "Invalid address range: %" PRIu8 " - %" PRIu8, startAddr, endAddr);
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
                ESP_LOGI(TAG, "Starting network scan from address %" PRIu8 " to %" PRIu8, startAddr, endAddr);

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

            bool ModbusDeviceManager::startPolling()
            {
                if (_pollingTaskHandle != nullptr)
                {
                    // Task already running
                    return true;
                }

                _pollingRunning = true;
                BaseType_t ret = xTaskCreate(
                    pollingTaskFunction,
                    "modbus_mgr_poll",
                    4096,  // Stack size
                    this,
                    POLLING_TASK_PRIORIT,     // Priority - higher than client polling
                    &_pollingTaskHandle);

                if (ret != pdPASS)
                {
                    ESP_LOGE(TAG, "Failed to create device manager polling task");
                    _pollingRunning = false;
                    return false;
                }

                ESP_LOGI(TAG, "Started centralized polling task");
                return true;
            }

            void ModbusDeviceManager::stopPolling()
            {
                if (_pollingTaskHandle != nullptr)
                {
                    _pollingRunning = false;
                    vTaskDelay(pdMS_TO_TICKS(100)); // Give task a chance to exit gracefully
                    vTaskDelete(_pollingTaskHandle);
                    _pollingTaskHandle = nullptr;
                    ESP_LOGI(TAG, "Stopped centralized polling task");
                }
            }

            void ModbusDeviceManager::setPollInterval(uint32_t defaultIntervalMs)
            {
                if (defaultIntervalMs < ModbusConstants::MIN_POLL_INTERVAL_MS)
                {
                    ESP_LOGW(TAG, "Poll interval %" PRIu32 " ms is too small, using minimum %" PRIu32 " ms",
                            defaultIntervalMs, ModbusConstants::MIN_POLL_INTERVAL_MS);
                    defaultIntervalMs = ModbusConstants::MIN_POLL_INTERVAL_MS;
                }
                
                _defaultPollIntervalMs = defaultIntervalMs;
                ESP_LOGI(TAG, "Set default polling interval to %" PRIu32 " ms", _defaultPollIntervalMs);
            }

            void ModbusDeviceManager::pollingTaskFunction(void *param)
            {
                ModbusDeviceManager *manager = static_cast<ModbusDeviceManager *>(param);
                
                // Initial delay to allow system to stabilize
                vTaskDelay(pdMS_TO_TICKS(500));
                
                ESP_LOGI(TAG, "Centralized polling task started");
                
                while (manager->_pollingRunning)
                {
                    manager->processPolling();
                    vTaskDelay(pdMS_TO_TICKS(20)); // Small delay between polling cycles
                }
                
                vTaskDelete(nullptr);
            }

            void ModbusDeviceManager::processPolling()
            {
                if (!_master || !_master->isConnected())
                {
                    // No connection, wait longer
                    vTaskDelay(pdMS_TO_TICKS(500));
                    return;
                }
                
                // Take mutex to safely access devices
                if (xSemaphoreTake(_devicesMutex, pdMS_TO_TICKS(100)) != pdTRUE)
                {
                    return;
                }
                
                uint32_t currentTime = xTaskGetTickCount();
                
                // Process each device
                for (auto &devicePair : _devices)
                {
                    auto device = devicePair.second;
                    
                    // Get poll configurations from device
                    const auto& pollConfig = device->getPollConfig();
                    
                    if (pollConfig.empty())
                    {
                        continue; // Skip devices with no polling configuration
                    }
                    
                    // Process each register for polling
                    for (const auto &regPair : pollConfig)
                    {
                        uint16_t address = regPair.first;
                        const auto &pollInfo = regPair.second;
                        
                        // Check if it's time to poll this register
                        uint32_t interval = pollInfo.intervalMs > 0 ? pollInfo.intervalMs : _defaultPollIntervalMs;
                        uint32_t timeSinceLastPoll = currentTime - pollInfo.lastPollTime;
                        
                        if (timeSinceLastPoll >= pdMS_TO_TICKS(interval))
                        {
                            // Release mutex during polling to avoid blocking other operations
                            xSemaphoreGive(_devicesMutex);
                            
                            // Poll the register and the device will update its cache and handle callbacks
                            device->pollRegister(address);
                            
                            // Re-acquire mutex
                            if (xSemaphoreTake(_devicesMutex, pdMS_TO_TICKS(100)) != pdTRUE)
                            {
                                ESP_LOGW(TAG, "Can't readquire the _devicesMutext for %s device", device->getName().c_str());
                                return; // Failed to reacquire mutex
                            }
                        }
                    }
                }
                
                xSemaphoreGive(_devicesMutex);
            }

        } // namespace modbus
    } // namespace net
} // namespace speed
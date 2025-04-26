#include "modbus_device_manager.hpp"
#include "esp_log.h"

namespace speed {
namespace net {
namespace modbus {

static const char* TAG = "ModbusDeviceManager";
static constexpr uint32_t SCAN_STACK_SIZE = 4096;
static constexpr uint32_t SCAN_TASK_PRIORITY = 5;
static constexpr uint32_t SCAN_TIMEOUT_MS = 200;

 // Create scan parameters structure
 struct ScanParams {
    ModbusDeviceManager* manager;
    uint8_t startAddr;
    uint8_t endAddr;
    DeviceDiscoveryCallback callback;
};

ModbusDeviceManager::ModbusDeviceManager(std::shared_ptr<ModbusMaster> master)
    : _master(master) {}

std::shared_ptr<ModbusClient> ModbusDeviceManager::addDevice(uint8_t address, const std::string& name) {
    if (hasDevice(address)) {
        ESP_LOGW(TAG, "Device with address %d already exists", address);
        return _devices[address];
    }

    auto client = std::make_shared<ModbusClient>(_master, address, name);
    _devices[address] = client;
    ESP_LOGI(TAG, "Added device %s with address %d", name.empty() ? "unnamed" : name.c_str(), address);
    return client;
}

void ModbusDeviceManager::removeDevice(uint8_t address) {
    auto it = _devices.find(address);
    if (it != _devices.end()) {
        ESP_LOGI(TAG, "Removing device %s with address %d", 
                 it->second->getName().empty() ? "unnamed" : it->second->getName().c_str(), address);
        _devices.erase(it);
    }
}

std::shared_ptr<ModbusClient> ModbusDeviceManager::getDevice(uint8_t address) {
    auto it = _devices.find(address);
    return (it != _devices.end()) ? it->second : nullptr;
}

std::vector<std::shared_ptr<ModbusClient>> ModbusDeviceManager::getAllDevices() const {
    std::vector<std::shared_ptr<ModbusClient>> devices;
    devices.reserve(_devices.size());
    for (const auto& pair : _devices) {
        devices.push_back(pair.second);
    }
    return devices;
}

void ModbusDeviceManager::scanNetwork(uint8_t startAddr, uint8_t endAddr, DeviceDiscoveryCallback callback) {
    if (_scanning) {
        ESP_LOGW(TAG, "Network scan already in progress");
        return;
    }

    if (startAddr > endAddr || endAddr > 247) {
        ESP_LOGE(TAG, "Invalid address range for network scan");
        return;
    }

    _scanning = true;


    auto params = new ScanParams{this, startAddr, endAddr, callback};

    BaseType_t ret = xTaskCreate(scanTaskFunction, "modbus_scan", SCAN_STACK_SIZE,
                                params, SCAN_TASK_PRIORITY, &_scanTask);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create scan task");
        delete params;
        _scanning = false;
    }
}

void ModbusDeviceManager::stopScan() {
    if (_scanning && _scanTask != nullptr) {
        _scanning = false;
        vTaskDelay(pdMS_TO_TICKS(100));  // Give task time to clean up
        // Task will delete itself
    }
}

void ModbusDeviceManager::scanTaskFunction(void* param) {
    auto params = static_cast<ScanParams*>(param);
    params->manager->processScanTask(params->startAddr, params->endAddr, params->callback);
    delete params;
    vTaskDelete(nullptr);
}

void ModbusDeviceManager::processScanTask(uint8_t startAddr, uint8_t endAddr, DeviceDiscoveryCallback callback) {
    ESP_LOGI(TAG, "Starting network scan from address %d to %d", startAddr, endAddr);

    for (uint8_t addr = startAddr; addr <= endAddr && _scanning; addr++) {
        bool present = testDevicePresence(addr);
        
        if (callback) {
            callback(addr, present);
        }

        if (present) {
            ESP_LOGI(TAG, "Found device at address %d", addr);
        }

        vTaskDelay(pdMS_TO_TICKS(50));  // Prevent overwhelming the network
    }

    ESP_LOGI(TAG, "Network scan completed");
    _scanning = false;
    _scanTask = nullptr;
}

bool ModbusDeviceManager::testDevicePresence(uint8_t address) {
    std::vector<uint8_t> response;
    bool success = false;

    // Try reading the first holding register
    auto callback = [&success](uint8_t slaveAddr, ModbusFunction function,
                             const std::vector<uint8_t>& data, TransactionStatus status) {
        success = (status == TransactionStatus::Success);
    };

    _master->readHoldingRegisters(address, 0, 1, callback);
    
    // Wait for response with timeout
    uint32_t startTime = xTaskGetTickCount();
    while ((xTaskGetTickCount() - startTime) < pdMS_TO_TICKS(SCAN_TIMEOUT_MS)) {
        if (success || !_scanning) break;
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    return success;
}

const ModbusStatistics& ModbusDeviceManager::getNetworkStatistics() const {
    return _master->getStatistics();
}

void ModbusDeviceManager::resetNetworkStatistics() {
    _master->resetStatistics();
}

void ModbusDeviceManager::disconnectAll() {
    _devices.clear();
    ESP_LOGI(TAG, "All devices disconnected");
}

} // namespace modbus
} // namespace net
} // namespace speed
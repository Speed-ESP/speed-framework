#include "modbus_client.hpp"
#include "esp_log.h"

namespace speed {
namespace net {
namespace modbus {

static const char* TAG = "ModbusClient";

ModbusClient::ModbusClient(std::shared_ptr<ModbusMaster> master, uint8_t slaveAddress, const std::string& name)
    : _master(master), _slaveAddress(slaveAddress), _name(name) {}

bool ModbusClient::readHoldingRegister(uint16_t address, ValueUpdateCallback callback) {
    // Check cache first
    if (callback) {
        auto cachedValue = getCachedValue(address);
        if (cachedValue.valid && isCacheValid(cachedValue)) {
            callback(cachedValue);
            return true;
        }
    }

    // Create callback wrapper to handle cache update
    auto responseCallback = [this, address, callback](uint8_t slaveAddr, ModbusFunction function,
                                                    const std::vector<uint8_t>& data, TransactionStatus status) {
        this->handleModbusResponse(address, data, status, callback);
    };

    return _master->readHoldingRegisters(_slaveAddress, address, 1, responseCallback);
}

bool ModbusClient::readMultipleHoldingRegisters(uint16_t startAddress, uint16_t quantity,
                                              std::function<void(const std::vector<ModbusValue>&)> callback) {
    auto responseCallback = [this, startAddress, quantity, callback](uint8_t slaveAddr, ModbusFunction function,
                                                                   const std::vector<uint8_t>& data, TransactionStatus status) {
        if (status == TransactionStatus::Success && data.size() >= 3) {
            std::vector<ModbusValue> values;
            values.reserve(quantity);
            
            for (uint16_t i = 0; i < quantity; i++) {
                uint16_t value = (data[3 + i * 2] << 8) | data[4 + i * 2];
                uint16_t address = startAddress + i;
                
                ModbusValue modbusValue{
                    .address = address,
                    .value = value,
                    .timestamp = xTaskGetTickCount(),
                    .valid = true
                };
                
                values.push_back(modbusValue);
                updateCache(address, value);
            }
            
            if (callback) {
                callback(values);
            }
        } else if (callback) {
            callback(std::vector<ModbusValue>());
        }
    };

    return _master->readHoldingRegisters(_slaveAddress, startAddress, quantity, responseCallback);
}

bool ModbusClient::writeHoldingRegister(uint16_t address, uint16_t value, ValueUpdateCallback callback) {
    auto responseCallback = [this, address, value, callback](uint8_t slaveAddr, ModbusFunction function,
                                                           const std::vector<uint8_t>& data, TransactionStatus status) {
        if (status == TransactionStatus::Success) {
            updateCache(address, value);
            if (callback) {
                ModbusValue modbusValue{
                    .address = address,
                    .value = value,
                    .timestamp = xTaskGetTickCount(),
                    .valid = true
                };
                callback(modbusValue);
            }
        } else if (callback) {
            ModbusValue modbusValue{
                .address = address,
                .value = 0,
                .timestamp = xTaskGetTickCount(),
                .valid = false
            };
            callback(modbusValue);
        }
    };

    return _master->writeSingleRegister(_slaveAddress, address, value, responseCallback);
}

bool ModbusClient::writeMultipleHoldingRegisters(uint16_t startAddress, const std::vector<uint16_t>& values,
                                               std::function<void(bool success)> callback) {
    auto responseCallback = [this, startAddress, values, callback](uint8_t slaveAddr, ModbusFunction function,
                                                                 const std::vector<uint8_t>& data, TransactionStatus status) {
        bool success = (status == TransactionStatus::Success);
        if (success) {
            for (size_t i = 0; i < values.size(); i++) {
                updateCache(startAddress + i, values[i]);
            }
        }
        if (callback) {
            callback(success);
        }
    };

    return _master->writeMultipleRegisters(_slaveAddress, startAddress, values, responseCallback);
}

bool ModbusClient::readCoil(uint16_t address, ValueUpdateCallback callback) {
    auto responseCallback = [this, address, callback](uint8_t slaveAddr, ModbusFunction function,
                                                    const std::vector<uint8_t>& data, TransactionStatus status) {
        if (status == TransactionStatus::Success && data.size() >= 3) {
            uint16_t value = (data[3] & 0x01) ? 1 : 0;
            updateCache(address, value);
            
            if (callback) {
                ModbusValue modbusValue{
                    .address = address,
                    .value = value,
                    .timestamp = xTaskGetTickCount(),
                    .valid = true
                };
                callback(modbusValue);
            }
        } else if (callback) {
            ModbusValue modbusValue{
                .address = address,
                .value = 0,
                .timestamp = xTaskGetTickCount(),
                .valid = false
            };
            callback(modbusValue);
        }
    };

    return _master->readCoils(_slaveAddress, address, 1, responseCallback);
}

bool ModbusClient::writeCoil(uint16_t address, bool value, ValueUpdateCallback callback) {
    auto responseCallback = [this, address, value, callback](uint8_t slaveAddr, ModbusFunction function,
                                                           const std::vector<uint8_t>& data, TransactionStatus status) {
        if (status == TransactionStatus::Success) {
            updateCache(address, value ? 1 : 0);
            if (callback) {
                ModbusValue modbusValue{
                    .address = address,
                    .value = value ? 1 : 0,
                    .timestamp = xTaskGetTickCount(),
                    .valid = true
                };
                callback(modbusValue);
            }
        } else if (callback) {
            ModbusValue modbusValue{
                .address = address,
                .value = 0,
                .timestamp = xTaskGetTickCount(),
                .valid = false
            };
            callback(modbusValue);
        }
    };

    return _master->writeSingleCoil(_slaveAddress, address, value, responseCallback);
}

ModbusValue ModbusClient::getCachedValue(uint16_t address) const {
    auto it = _cache.find(address);
    if (it != _cache.end() && isCacheValid(it->second)) {
        return it->second;
    }
    return ModbusValue{
        .address = address,
        .value = 0,
        .timestamp = 0,
        .valid = false
    };
}

void ModbusClient::clearCache() {
    _cache.clear();
}

void ModbusClient::enablePolling(uint16_t address, uint32_t intervalMs, ValueUpdateCallback callback) {
    PollInfo pollInfo{
        .intervalMs = intervalMs,
        .lastPollTime = 0,
        .callback = callback
    };
    _pollConfig[address] = pollInfo;
    
    // Initial read
    readHoldingRegister(address, callback);
}

void ModbusClient::disablePolling(uint16_t address) {
    _pollConfig.erase(address);
}

void ModbusClient::setPollInterval(uint16_t address, uint32_t intervalMs) {
    auto it = _pollConfig.find(address);
    if (it != _pollConfig.end()) {
        it->second.intervalMs = intervalMs;
    }
}

void ModbusClient::updateCache(uint16_t address, uint16_t value) {
    ModbusValue modbusValue{
        .address = address,
        .value = value,
        .timestamp = xTaskGetTickCount(),
        .valid = true
    };
    _cache[address] = modbusValue;
}

bool ModbusClient::isCacheValid(const ModbusValue& value) const {
    if (!value.valid) return false;
    uint32_t age = xTaskGetTickCount() - value.timestamp;
    return age <= pdMS_TO_TICKS(_maxCacheAge);
}

void ModbusClient::handleModbusResponse(uint16_t address, const std::vector<uint8_t>& data,
                                      TransactionStatus status, ValueUpdateCallback callback) {
    if (status == TransactionStatus::Success && data.size() >= 3) {
        uint16_t value = (data[3] << 8) | data[4];
        updateCache(address, value);
        
        if (callback) {
            ModbusValue modbusValue{
                .address = address,
                .value = value,
                .timestamp = xTaskGetTickCount(),
                .valid = true
            };
            callback(modbusValue);
        }
    } else if (callback) {
        ModbusValue modbusValue{
            .address = address,
            .value = 0,
            .timestamp = xTaskGetTickCount(),
            .valid = false
        };
        callback(modbusValue);
    }
}

} // namespace modbus
} // namespace net
} // namespace speed
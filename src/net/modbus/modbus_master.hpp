#pragma once

#include "modbus_config.hpp"
#include "modbus_defs.hpp"
#include "transport/modbus_transport.hpp"
#include <cstdint>
#include <vector>
#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

namespace speed {
namespace net {
namespace modbus {

using ModbusCallback = std::function<void(uint8_t slaveAddr, ModbusFunction function, 
                                        const std::vector<uint8_t>& data, TransactionStatus status)>;

class ModbusMaster {
public:
    ModbusMaster(std::shared_ptr<ModbusTransport> transport, const ModbusConfig& config = ModbusConfig());
    ~ModbusMaster();

    bool begin();
    void stop();
    void setGlobalCallback(ModbusCallback callback);

    // High-level Modbus operations
    bool readHoldingRegisters(uint8_t slaveAddr, uint16_t startAddr, uint16_t quantity, ModbusCallback callback = nullptr);
    bool readInputRegisters(uint8_t slaveAddr, uint16_t startAddr, uint16_t quantity, ModbusCallback callback = nullptr);
    bool readCoils(uint8_t slaveAddr, uint16_t startAddr, uint16_t quantity, ModbusCallback callback = nullptr);
    bool readDiscreteInputs(uint8_t slaveAddr, uint16_t startAddr, uint16_t quantity, ModbusCallback callback = nullptr);
    bool writeSingleRegister(uint8_t slaveAddr, uint16_t regAddr, uint16_t value, ModbusCallback callback = nullptr);
    bool writeSingleCoil(uint8_t slaveAddr, uint16_t coilAddr, bool value, ModbusCallback callback = nullptr);
    bool writeMultipleRegisters(uint8_t slaveAddr, uint16_t startAddr, const std::vector<uint16_t>& values, ModbusCallback callback = nullptr);
    bool writeMultipleCoils(uint8_t slaveAddr, uint16_t startAddr, const std::vector<bool>& values, ModbusCallback callback = nullptr);

    // Low-level access (for advanced users)
    bool sendRequest(uint8_t slaveAddr, ModbusFunction function, uint16_t startAddr, 
                    uint16_t quantity, const std::vector<uint8_t>& data = std::vector<uint8_t>(),
                    ModbusCallback callback = nullptr);

    // Diagnostics and management
    const ModbusStatistics& getStatistics() const { return _statistics; }
    void resetStatistics() { _statistics.reset(); }
    void setConfig(const ModbusConfig& config);
    const ModbusConfig& getConfig() const { return _config; }
    bool isConnected() const { return _transport && _transport->isConnected(); }

private:
    std::shared_ptr<ModbusTransport> _transport;
    TaskHandle_t _taskHandle;
    QueueHandle_t _requestQueue;
    ModbusCallback _globalCallback;
    bool _running;
    ModbusConfig _config;
    ModbusStatistics _statistics;
    std::map<uint16_t, ModbusTransaction> _pendingTransactions;
    std::mutex _transactionMutex;
    uint16_t _nextTransactionId{1};
    SemaphoreHandle_t _transactionSemaphore;

    static void modbusTask(void* parameter);
    void processModbusTask();
    bool processRequest(ModbusTransaction& transaction);
    bool validateRequest(const ModbusTransaction& transaction) const;
    bool handleModbusException(uint8_t exceptionCode, ModbusTransaction& transaction);
    uint16_t calculateCRC(const uint8_t* data, size_t length) const;
    uint16_t getNextTransactionId();
    void cleanupTimedOutTransactions();
    bool retryTransaction(ModbusTransaction& transaction);
    void updateStatistics(const ModbusTransaction& transaction);
    size_t calculateExpectedResponseLength(const ModbusTransaction& transaction) const;
};

} // namespace modbus
} // namespace net
} // namespace speed
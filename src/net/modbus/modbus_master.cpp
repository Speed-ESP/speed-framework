#include "modbus_master.hpp"
#include "esp_log.h"
#include <algorithm>

namespace speed {
namespace net {
namespace modbus {

static const char* TAG = "ModbusMaster";

ModbusMaster::ModbusMaster(std::shared_ptr<ModbusTransport> transport, const ModbusConfig& config)
    : _transport(transport), _taskHandle(nullptr), _requestQueue(nullptr),
      _globalCallback(nullptr), _running(false), _config(config) {
    _transactionSemaphore = xSemaphoreCreateMutex();
}

ModbusMaster::~ModbusMaster() {
    stop();
    if (_transactionSemaphore) {
        vSemaphoreDelete(_transactionSemaphore);
    }
}

bool ModbusMaster::begin() {
    if (!_transport || !_transport->begin()) {
        ESP_LOGE(TAG, "Failed to initialize transport");
        return false;
    }

    _requestQueue = xQueueCreate(_config.queueSize, sizeof(ModbusTransaction));
    if (_requestQueue == nullptr) {
        ESP_LOGE(TAG, "Failed to create request queue");
        return false;
    }

    _running = true;
    BaseType_t ret = xTaskCreate(modbusTask, "modbus_task", _config.stackSize,
                                this, _config.taskPriority, &_taskHandle);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create modbus task");
        _running = false;
        vQueueDelete(_requestQueue);
        _requestQueue = nullptr;
        return false;
    }

    return true;
}

void ModbusMaster::stop() {
    if (_running) {
        _running = false;
        if (_taskHandle != nullptr) {
            vTaskDelete(_taskHandle);
            _taskHandle = nullptr;
        }
        if (_requestQueue != nullptr) {
            vQueueDelete(_requestQueue);
            _requestQueue = nullptr;
        }
        if (_transport) {
            _transport->stop();
        }
        
        std::lock_guard<std::mutex> lock(_transactionMutex);
        _pendingTransactions.clear();
    }
}

bool ModbusMaster::readHoldingRegisters(uint8_t slaveAddr, uint16_t startAddr, uint16_t quantity, ModbusCallback callback) {
    if (quantity > ModbusConstants::MAX_REGISTERS_PER_REQUEST) {
        ESP_LOGE(TAG, "Quantity exceeds maximum allowed registers (%d > %d)", 
                 quantity, ModbusConstants::MAX_REGISTERS_PER_REQUEST);
        return false;
    }
    return sendRequest(slaveAddr, ModbusFunction::ReadHoldingRegisters, startAddr, quantity, {}, callback);
}

bool ModbusMaster::writeSingleRegister(uint8_t slaveAddr, uint16_t regAddr, uint16_t value, ModbusCallback callback) {
    std::vector<uint8_t> data = {static_cast<uint8_t>(value >> 8), static_cast<uint8_t>(value & 0xFF)};
    return sendRequest(slaveAddr, ModbusFunction::WriteSingleRegister, regAddr, 1, data, callback);
}

bool ModbusMaster::sendRequest(uint8_t slaveAddr, ModbusFunction function, uint16_t startAddr, 
                             uint16_t quantity, const std::vector<uint8_t>& data, ModbusCallback callback) {
    if (!_running || !_transport->isConnected()) return false;

    ModbusTransaction transaction;
    transaction.transactionId = getNextTransactionId();
    transaction.slaveAddr = slaveAddr;
    transaction.function = function;
    transaction.startAddress = startAddr;
    transaction.quantity = quantity;
    transaction.data = data;
    transaction.status = TransactionStatus::Pending;
    transaction.retryCount = 0;
    transaction.timestamp = xTaskGetTickCount();

    if (!validateRequest(transaction)) {
        return false;
    }

    if (xQueueSend(_requestQueue, &transaction, pdMS_TO_TICKS(_config.queueTimeoutMs)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to queue Modbus request");
        return false;
    }

    if (callback) {
        std::lock_guard<std::mutex> lock(_transactionMutex);
        transaction.data.clear(); // Clear data to save memory in pending transactions
        _pendingTransactions[transaction.transactionId] = transaction;
    }
    
    return true;
}

void ModbusMaster::processModbusTask() {
    ModbusTransaction transaction;
    TickType_t lastCleanupTime = xTaskGetTickCount();
    
    while (_running) {
        if (xQueueReceive(_requestQueue, &transaction, pdMS_TO_TICKS(_config.queueTimeoutMs)) == pdTRUE) {
            processRequest(transaction);
        }

        // Periodic cleanup of timed-out transactions
        if ((xTaskGetTickCount() - lastCleanupTime) > pdMS_TO_TICKS(1000)) {
            cleanupTimedOutTransactions();
            lastCleanupTime = xTaskGetTickCount();
        }
    }
}

bool ModbusMaster::processRequest(ModbusTransaction& transaction) {
    std::vector<uint8_t> requestData;
    requestData.push_back(transaction.slaveAddr);
    requestData.push_back(static_cast<uint8_t>(transaction.function));
    requestData.push_back(transaction.startAddress >> 8);
    requestData.push_back(transaction.startAddress & 0xFF);
    requestData.push_back(transaction.quantity >> 8);
    requestData.push_back(transaction.quantity & 0xFF);

    // Add additional data for write operations
    requestData.insert(requestData.end(), transaction.data.begin(), transaction.data.end());

    uint16_t crc = calculateCRC(requestData.data(), requestData.size());
    requestData.push_back(crc & 0xFF);
    requestData.push_back(crc >> 8);

    _transport->flush();

    if (!_transport->send(requestData.data(), requestData.size())) {
        ESP_LOGE(TAG, "Failed to send request");
        transaction.status = TransactionStatus::ConnectionError;
        updateStatistics(transaction);
        return retryTransaction(transaction);
    }

    _statistics.messagesSent++;

    // Calculate expected response length based on function code
    size_t expectedLength = calculateExpectedResponseLength(transaction);
    std::vector<uint8_t> response(expectedLength);

    if (!_transport->receive(response.data(), expectedLength, _config.responseTimeoutMs)) {
        ESP_LOGE(TAG, "Failed to receive response");
        transaction.status = TransactionStatus::Timeout;
        updateStatistics(transaction);
        return retryTransaction(transaction);
    }

    _statistics.messagesReceived++;

    // Check for Modbus exception response
    if ((response[1] & 0x80) == 0x80) {
        return handleModbusException(response[2], transaction);
    }

    // Validate CRC
    uint16_t respCRC = (response[expectedLength - 1] << 8) | response[expectedLength - 2];
    uint16_t calcCRC = calculateCRC(response.data(), expectedLength - 2);
    if (respCRC != calcCRC) {
        ESP_LOGE(TAG, "CRC mismatch: received 0x%X, calculated 0x%X", respCRC, calcCRC);
        transaction.status = TransactionStatus::CrcError;
        updateStatistics(transaction);
        return retryTransaction(transaction);
    }

    transaction.status = TransactionStatus::Success;
    transaction.data = response;
    updateStatistics(transaction);

    // Execute callbacks
    if (_globalCallback) {
        _globalCallback(transaction.slaveAddr, transaction.function, response, transaction.status);
    }

    {
        std::lock_guard<std::mutex> lock(_transactionMutex);
        auto it = _pendingTransactions.find(transaction.transactionId);
        if (it != _pendingTransactions.end()) {
            if (it->second.callback) {
                it->second.callback(transaction.slaveAddr, transaction.function, response, transaction.status);
            }
            _pendingTransactions.erase(it);
        }
    }

    return true;
}

bool ModbusMaster::validateRequest(const ModbusTransaction& transaction) const {
    if (transaction.slaveAddr > 247 && transaction.slaveAddr != ModbusConstants::BROADCAST_ADDRESS) {
        ESP_LOGE(TAG, "Invalid slave address: %d", transaction.slaveAddr);
        return false;
    }

    switch (transaction.function) {
        case ModbusFunction::ReadHoldingRegisters:
        case ModbusFunction::ReadInputRegisters:
            if (transaction.quantity > ModbusConstants::MAX_REGISTERS_PER_REQUEST) {
                ESP_LOGE(TAG, "Quantity exceeds maximum allowed registers");
                return false;
            }
            break;
        case ModbusFunction::ReadCoils:
        case ModbusFunction::ReadDiscreteInputs:
            if (transaction.quantity > ModbusConstants::MAX_COILS_PER_REQUEST) {
                ESP_LOGE(TAG, "Quantity exceeds maximum allowed coils");
                return false;
            }
            break;
        default:
            break;
    }

    return true;
}

uint16_t ModbusMaster::getNextTransactionId() {
    if (xSemaphoreTake(_transactionSemaphore, portMAX_DELAY) == pdTRUE) {
        uint16_t id = _nextTransactionId++;
        if (_nextTransactionId == 0) _nextTransactionId = 1;
        xSemaphoreGive(_transactionSemaphore);
        return id;
    }
    return 0;
}

void ModbusMaster::cleanupTimedOutTransactions() {
    std::lock_guard<std::mutex> lock(_transactionMutex);
    auto now = xTaskGetTickCount();
    
    for (auto it = _pendingTransactions.begin(); it != _pendingTransactions.end();) {
        if ((now - it->second.timestamp) > pdMS_TO_TICKS(_config.responseTimeoutMs)) {
            if (it->second.callback) {
                it->second.callback(it->second.slaveAddr, it->second.function, {}, TransactionStatus::Timeout);
            }
            it = _pendingTransactions.erase(it);
        } else {
            ++it;
        }
    }
}

bool ModbusMaster::retryTransaction(ModbusTransaction& transaction) {
    if (transaction.retryCount >= ModbusConstants::MAX_RETRIES) {
        return false;
    }
    
    transaction.retryCount++;
    _statistics.retries++;
    transaction.timestamp = xTaskGetTickCount();
    
    return xQueueSendToFront(_requestQueue, &transaction, 0) == pdTRUE;
}

void ModbusMaster::updateStatistics(const ModbusTransaction& transaction) {
    switch (transaction.status) {
        case TransactionStatus::Success:
            _statistics.successfulTransactions++;
            break;
        case TransactionStatus::Timeout:
            _statistics.timeouts++;
            _statistics.failedTransactions++;
            break;
        case TransactionStatus::CrcError:
            _statistics.crcErrors++;
            _statistics.failedTransactions++;
            break;
        case TransactionStatus::ExceptionReceived:
            _statistics.exceptionResponses++;
            _statistics.failedTransactions++;
            break;
        default:
            _statistics.failedTransactions++;
            break;
    }
}

size_t ModbusMaster::calculateExpectedResponseLength(const ModbusTransaction& transaction) const {
    // Check for exception response first
    if (transaction.data.size() >= 2 && (transaction.data[1] & 0x80)) {
        return _transport->getType() == ModbusTransportType::TCP ? 
               ModbusConstants::TCP_EXCEPTION_LENGTH : 
               ModbusConstants::RTU_EXCEPTION_LENGTH;
    }

    // Base length includes transport header + function code
    size_t baseLength = m_transport->getType() == ModbusTransportType::TCP ? 
                       ModbusConstants::TCP_HEADER_SIZE : 
                       ModbusConstants::RTU_HEADER_SIZE;
    
    // Add transport footer (CRC for RTU)
    size_t footerLength = m_transport->getType() == ModbusTransportType::TCP ? 
                         0 : ModbusConstants::RTU_CRC_SIZE;

    // Calculate PDU length based on function code
    size_t pduLength = 1;  // Function code
    switch (transaction.function) {
        case ModbusFunction::ReadCoils:
        case ModbusFunction::ReadDiscreteInputs:
            // Byte count + data bytes (8 coils per byte, rounded up)
            pduLength += 1 + ((transaction.quantity + 7) / 8);
            break;
            
        case ModbusFunction::ReadHoldingRegisters:
        case ModbusFunction::ReadInputRegisters:
            // Byte count + data bytes (2 bytes per register)
            pduLength += 1 + (transaction.quantity * 2);
            break;
            
        case ModbusFunction::WriteSingleCoil:
        case ModbusFunction::WriteSingleRegister:
            // Address + value
            pduLength += 4;
            break;
            
        case ModbusFunction::WriteMultipleCoils:
        case ModbusFunction::WriteMultipleRegisters:
            // Address + quantity
            pduLength += 4;
            break;
            
        case ModbusFunction::ReadWriteMultipleRegisters:
            // Byte count + read data bytes
            pduLength += 1 + (transaction.quantity * 2);
            break;
            
        case ModbusFunction::Diagnostics:
            // Sub-function + data
            pduLength += transaction.data.size();
            break;
            
        default:
            // Unknown function code, use minimum response size
            pduLength += 1;
            break;
    }

    return baseLength + pduLength + footerLength;
}

bool ModbusMaster::handleModbusException(uint8_t exceptionCode, ModbusTransaction& transaction) {
    ModbusError error = static_cast<ModbusError>(exceptionCode);
    const char* errorStr;
    
    switch (error) {
        case ModbusError::IllegalFunction:
            errorStr = "Illegal function";
            break;
        case ModbusError::IllegalDataAddress:
            errorStr = "Illegal data address";
            break;
        case ModbusError::IllegalDataValue:
            errorStr = "Illegal data value";
            break;
        case ModbusError::SlaveDeviceFailure:
            errorStr = "Slave device failure";
            break;
        case ModbusError::Acknowledge:
            errorStr = "Acknowledge";
            break;
        case ModbusError::SlaveDeviceBusy:
            errorStr = "Slave device busy";
            break;
        default:
            errorStr = "Unknown error";
            break;
    }
    
    ESP_LOGW(TAG, "Modbus exception received from slave %d: %s (0x%02X)",
             transaction.slaveAddr, errorStr, exceptionCode);
             
    transaction.status = TransactionStatus::ExceptionReceived;
    return false;
}

} // namespace modbus
} // namespace net
} // namespace speed
#include "esp_log.h"
#include <algorithm>
#include <inttypes.h>

#include <net/modbus/modbus_master.hpp>
#include <net/modbus/modbus_utils.hpp>
#include "modbus_master.hpp"
namespace speed
{
    namespace net
    {
        namespace modbus
        {

            static const char *TAG = "ModbusMaster";

            ModbusMaster::ModbusMaster(std::shared_ptr<ModbusTransport> transport, const ModbusConfig &config)
                : _transport(transport), _packager(nullptr), _taskHandle(nullptr), _requestQueue(nullptr),
                  _globalCallback(nullptr), _running(false), _config(config)
            {
                _transactionSemaphore = xSemaphoreCreateMutex();
                
                // Initialize the default packager for this transport
                initializeDefaultPackager();
            }

            ModbusMaster::~ModbusMaster()
            {
                stop();
                if (_transactionSemaphore)
                {
                    vSemaphoreDelete(_transactionSemaphore);
                }
            }

            bool ModbusMaster::begin()
            {
                if (!_transport || !_transport->begin())
                {
                    ESP_LOGE(TAG, "Failed to initialize transport");
                    return false;
                }

                _requestQueue = xQueueCreate(_config.queueSize, sizeof(ModbusTransaction));
                if (_requestQueue == nullptr)
                {
                    ESP_LOGE(TAG, "Failed to create request queue");
                    return false;
                }

                _running = true;
                BaseType_t ret = xTaskCreate(ModbusMaster::modbusTask, "modbus_task", _config.stackSize,
                                             this, _config.taskPriority, &_taskHandle);
                if (ret != pdPASS)
                {
                    ESP_LOGE(TAG, "Failed to create modbus task");
                    _running = false;
                    vQueueDelete(_requestQueue);
                    _requestQueue = nullptr;
                    return false;
                }
                ESP_LOGD(TAG, "Processing modbus task crated with priority: %lu, and stack: %lu", _config.taskPriority, _config.stackSize);
                

                return true;
            }

            void ModbusMaster::stop()
            {
                ESP_LOGI(TAG, "STOPPING MODBUS!!!!");
                if (_running)
                {
                    _running = false;
                    if (_taskHandle != nullptr)
                    {
                        vTaskDelete(_taskHandle);
                        _taskHandle = nullptr;
                    }
                    if (_requestQueue != nullptr)
                    {
                        vQueueDelete(_requestQueue);
                        _requestQueue = nullptr;
                    }
                    if (_transport)
                    {
                        _transport->stop();
                    }

                    std::lock_guard<std::mutex> lock(_transactionMutex);
                    _pendingTransactions.clear();
                }
            }

            void ModbusMaster::setGlobalCallback(ModbusCallback callback)
            {
                _globalCallback = callback;
            }
            bool ModbusMaster::readHoldingRegisters(uint8_t slaveAddr, uint16_t startAddr, uint16_t quantity, ModbusCallback callback)
            {
                if (quantity > ModbusConstants::MAX_REGISTERS_PER_REQUEST)
                {
                    ESP_LOGE(TAG, "Quantity exceeds maximum allowed registers (%" PRIu16 " > %" PRIu16 ")",
                             quantity, ModbusConstants::MAX_REGISTERS_PER_REQUEST);
                    return false;
                }
                return sendRequest(slaveAddr, ModbusFunction::ReadHoldingRegisters, startAddr, quantity, {}, callback);
            }

            bool ModbusMaster::readInputRegisters(uint8_t slaveAddr, uint16_t startAddr, uint16_t quantity, ModbusCallback callback)
            {
                if (quantity > ModbusConstants::MAX_REGISTERS_PER_REQUEST) {
                    ESP_LOGE(TAG, "Quantity exceeds maximum allowed registers (%" PRIu16 " > %" PRIu16 ")",
                             quantity, ModbusConstants::MAX_REGISTERS_PER_REQUEST);
                    throw std::invalid_argument("Quantity exceeds maximum allowed registers");
                }
                
                if (slaveAddr > ModbusConstants::MAX_DEVICE_ADDRESS) {
                    ESP_LOGE(TAG, "Invalid slave address: %" PRIu8, slaveAddr);
                    throw std::invalid_argument("Invalid slave address");
                }

                ESP_LOGD(TAG, "Reading %" PRIu16 " input registers from address %" PRIu16 " of slave %" PRIu8,
                         quantity, startAddr, slaveAddr);
                         
                return sendRequest(slaveAddr, ModbusFunction::ReadInputRegisters, startAddr, quantity, {}, callback);
            }

            bool ModbusMaster::readCoils(uint8_t slaveAddr, uint16_t startAddr, uint16_t quantity, ModbusCallback callback)
            {
                if (quantity > ModbusConstants::MAX_COILS_PER_REQUEST) {
                    ESP_LOGE(TAG, "Quantity exceeds maximum allowed coils (%" PRIu16 " > %" PRIu16 ")",
                             quantity, ModbusConstants::MAX_COILS_PER_REQUEST);
                    throw std::invalid_argument("Quantity exceeds maximum allowed coils");
                }
                
                if (slaveAddr > ModbusConstants::MAX_DEVICE_ADDRESS) {
                    ESP_LOGE(TAG, "Invalid slave address: %" PRIu8, slaveAddr);
                    throw std::invalid_argument("Invalid slave address");
                }

                ESP_LOGD(TAG, "Reading %" PRIu16 " coils from address %" PRIu16 " of slave %" PRIu8,
                         quantity, startAddr, slaveAddr);
                         
                return sendRequest(slaveAddr, ModbusFunction::ReadCoils, startAddr, quantity, {}, callback);
            }

            bool ModbusMaster::readDiscreteInputs(uint8_t slaveAddr, uint16_t startAddr, uint16_t quantity, ModbusCallback callback)
            {
                if (quantity > ModbusConstants::MAX_COILS_PER_REQUEST) {
                    ESP_LOGE(TAG, "Quantity exceeds maximum allowed discrete inputs (%" PRIu16 " > %" PRIu16 ")",
                             quantity, ModbusConstants::MAX_COILS_PER_REQUEST);
                    throw std::invalid_argument("Quantity exceeds maximum allowed discrete inputs");
                }
                
                if (slaveAddr > ModbusConstants::MAX_DEVICE_ADDRESS) {
                    ESP_LOGE(TAG, "Invalid slave address: %" PRIu8, slaveAddr);
                    throw std::invalid_argument("Invalid slave address");
                }

                ESP_LOGD(TAG, "Reading %" PRIu16 " discrete inputs from address %" PRIu16 " of slave %" PRIu8,
                         quantity, startAddr, slaveAddr);
                         
                return sendRequest(slaveAddr, ModbusFunction::ReadDiscreteInputs, startAddr, quantity, {}, callback);
            }

            bool ModbusMaster::writeSingleRegister(uint8_t slaveAddr, uint16_t regAddr, uint16_t value, ModbusCallback callback)
            {
                std::vector<uint8_t> data = {static_cast<uint8_t>(value >> 8), static_cast<uint8_t>(value & 0xFF)};
                return sendRequest(slaveAddr, ModbusFunction::WriteSingleRegister, regAddr, 1, data, callback);
            }

            bool ModbusMaster::writeSingleCoil(uint8_t slaveAddr, uint16_t coilAddr, bool value, ModbusCallback callback)
            {
                if (slaveAddr > ModbusConstants::MAX_DEVICE_ADDRESS && slaveAddr != ModbusConstants::BROADCAST_ADDRESS) {
                    ESP_LOGE(TAG, "Invalid slave address: %" PRIu8, slaveAddr);
                    throw std::invalid_argument("Invalid slave address");
                }

                ESP_LOGD(TAG, "Writing coil at address %" PRIu16 " of slave %" PRIu8 " with value %d",
                         coilAddr, slaveAddr, value);

                // In Modbus protocol, coil value is represented as 0xFF00 for ON and 0x0000 for OFF
                uint16_t modbusValue = value ? 0xFF00 : 0x0000;
                std::vector<uint8_t> data = {static_cast<uint8_t>(modbusValue >> 8), static_cast<uint8_t>(modbusValue & 0xFF)};
                
                return sendRequest(slaveAddr, ModbusFunction::WriteSingleCoil, coilAddr, 1, data, callback);
            }

            bool ModbusMaster::writeMultipleRegisters(uint8_t slaveAddr, uint16_t startAddr, const std::vector<uint16_t> &values, ModbusCallback callback)
            {
                if (values.empty() || values.size() > ModbusConstants::MAX_REGISTERS_PER_REQUEST) {
                    ESP_LOGE(TAG, "Invalid number of registers to write (%" PRIu16 ")", static_cast<uint16_t>(values.size()));
                    throw std::invalid_argument("Invalid number of registers");
                }

                if (slaveAddr > ModbusConstants::MAX_DEVICE_ADDRESS && slaveAddr != ModbusConstants::BROADCAST_ADDRESS) {
                    ESP_LOGE(TAG, "Invalid slave address: %" PRIu8, slaveAddr);
                    throw std::invalid_argument("Invalid slave address");
                }

                ESP_LOGD(TAG, "Writing %" PRIu16 " registers starting at address %" PRIu16 " of slave %" PRIu8,
                         static_cast<uint16_t>(values.size()), startAddr, slaveAddr);

                // Prepare data: byte count + register values
                std::vector<uint8_t> data;
                data.push_back(static_cast<uint8_t>(values.size() * 2)); // Byte count

                // Add register values
                for (uint16_t value : values) {
                    data.push_back(static_cast<uint8_t>(value >> 8));
                    data.push_back(static_cast<uint8_t>(value & 0xFF));
                }

                return sendRequest(slaveAddr, ModbusFunction::WriteMultipleRegisters, startAddr, static_cast<uint16_t>(values.size()), data, callback);
            }

            bool ModbusMaster::writeMultipleCoils(uint8_t slaveAddr, uint16_t startAddr, const std::vector<bool> &values, ModbusCallback callback)
            {
                if (values.empty() || values.size() > ModbusConstants::MAX_COILS_PER_REQUEST) {
                    ESP_LOGE(TAG, "Invalid number of coils to write (%" PRIu16 ")", static_cast<uint16_t>(values.size()));
                    throw std::invalid_argument("Invalid number of coils");
                }

                if (slaveAddr > ModbusConstants::MAX_DEVICE_ADDRESS && slaveAddr != ModbusConstants::BROADCAST_ADDRESS) {
                    ESP_LOGE(TAG, "Invalid slave address: %" PRIu8, slaveAddr);
                    throw std::invalid_argument("Invalid slave address");
                }

                ESP_LOGD(TAG, "Writing %" PRIu16 " coils starting at address %" PRIu16 " of slave %" PRIu8,
                         static_cast<uint16_t>(values.size()), startAddr, slaveAddr);

                // Calculate number of bytes needed to hold all coils (8 coils per byte)
                size_t byteCount = (values.size() + 7) / 8;
                std::vector<uint8_t> data;
                data.push_back(static_cast<uint8_t>(byteCount)); // Byte count

                // Pack coils into bytes
                uint8_t currentByte = 0;
                uint8_t bitIndex = 0;

                for (size_t i = 0; i < values.size(); i++) {
                    if (values[i]) {
                        currentByte |= (1 << bitIndex);
                    }
                    bitIndex++;
                    
                    if (bitIndex == 8 || i == values.size() - 1) {
                        data.push_back(currentByte);
                        currentByte = 0;
                        bitIndex = 0;
                    }
                }

                return sendRequest(slaveAddr, ModbusFunction::WriteMultipleCoils, startAddr, static_cast<uint16_t>(values.size()), data, callback);
            }

            bool ModbusMaster::sendRequest(uint8_t slaveAddr, ModbusFunction function, uint16_t startAddr,
                                           uint16_t quantity, const std::vector<uint8_t> &data, ModbusCallback callback)
            {
                if (!_running || !_transport->isConnected())
                    return false;

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

                if (!validateRequest(transaction))
                {
                    return false;
                }
                ESP_LOGD(TAG, "Queuing request with transacitonId %u to device %d", transaction.transactionId, transaction.slaveAddr);
                if (xQueueSend(_requestQueue, &transaction, pdMS_TO_TICKS(_config.queueTimeoutMs)) != pdTRUE)
                {
                    ESP_LOGE(TAG, "Failed to queue Modbus request");
                    return false;
                }

                if (callback)
                {
                    std::lock_guard<std::mutex> lock(_transactionMutex);
                    transaction.data.clear(); // Clear data to save memory in pending transactions
                    _pendingTransactions[transaction.transactionId] = transaction;
                }

                return true;
            }

            void ModbusMaster::modbusTask(void *parameter)
            {
                ModbusMaster* self = (ModbusMaster*)parameter;
                ModbusTransaction transaction;
                TickType_t lastCleanupTime = xTaskGetTickCount();
                ESP_LOGI(TAG, "Starting processing request task, is running? %d", self->_running.load());
                while (self->_running)
                {
                    ESP_LOGD(TAG, "Dequeing from request queue");
                    if (xQueueReceive(self->_requestQueue, &transaction, pdMS_TO_TICKS(self->_config.queueTimeoutMs)) == pdTRUE)
                    {
                        self->processRequest(transaction);
                    }
                    ESP_LOGD(TAG, "Couldn't read from the request queue queue");
                    // Periodic cleanup of timed-out transactions
                    if ((xTaskGetTickCount() - lastCleanupTime) > pdMS_TO_TICKS(1000))
                    {
                        self->cleanupTimedOutTransactions();
                        lastCleanupTime = xTaskGetTickCount();
                    }
                }

                ESP_LOGI(TAG, "Exiting processing request task");
            }

            bool ModbusMaster::processRequest(ModbusTransaction &transaction)
            {
                ESP_LOGD(TAG,"Processing request id: %d, to device: %d", transaction.transactionId, transaction.slaveAddr);
                
                // Use the packager to format the request
                std::vector<uint8_t> requestData;
                if (!_packager->packageRequest(transaction, requestData))
                {
                    ESP_LOGE(TAG, "Failed to package request");
                    transaction.status = TransactionStatus::InvalidResponse;
                    updateStatistics(transaction);
                    return false;
                }

                _transport->flush();
                if (!_transport->send(requestData.data(), requestData.size()))
                {
                    ESP_LOGE(TAG, "Failed to send request");
                    transaction.status = TransactionStatus::ConnectionError;
                    updateStatistics(transaction);
                    return retryTransaction(transaction);
                }

                _statistics.messagesSent++;

                // Calculate expected response length based on function code
                size_t expectedLength = _packager->calculateExpectedResponseLength(transaction);
                std::vector<uint8_t> responseData(expectedLength);

                if (!_transport->receive(responseData.data(), expectedLength, _config.responseTimeoutMs))
                {
                    ESP_LOGE(TAG, "Failed to receive response");
                    transaction.status = TransactionStatus::Timeout;
                    updateStatistics(transaction);
                    return retryTransaction(transaction);
                }

                _statistics.messagesReceived++;

                // Parse response using the packager
                if (!_packager->parseResponse(responseData, transaction))
                {
                    ESP_LOGE(TAG, "Failed to parse response");
                    transaction.status = TransactionStatus::InvalidResponse;
                    updateStatistics(transaction);
                    return retryTransaction(transaction);
                }

                updateStatistics(transaction);

                // Execute callbacks
                if (_globalCallback)
                {
                    _globalCallback(transaction.slaveAddr, transaction.function, transaction.data, transaction.status);
                }

                {
                    std::lock_guard<std::mutex> lock(_transactionMutex);
                    auto it = _pendingTransactions.find(transaction.transactionId);
                    if (it != _pendingTransactions.end())
                    {
                        if (it->second.callback)
                        {
                            it->second.callback(transaction.slaveAddr, transaction.function, transaction.data, transaction.status);
                        }
                        _pendingTransactions.erase(it);
                    }
                }

                return true;
            }

            bool ModbusMaster::validateRequest(const ModbusTransaction &transaction) const
            {
                if (transaction.slaveAddr > 247 && transaction.slaveAddr != ModbusConstants::BROADCAST_ADDRESS)
                {
                    ESP_LOGE(TAG, "Invalid slave address: %" PRIu8, transaction.slaveAddr);
                    return false;
                }

                switch (transaction.function)
                {
                case ModbusFunction::ReadHoldingRegisters:
                case ModbusFunction::ReadInputRegisters:
                    if (transaction.quantity > ModbusConstants::MAX_REGISTERS_PER_REQUEST)
                    {
                        ESP_LOGE(TAG, "Quantity exceeds maximum allowed registers");
                        return false;
                    }
                    break;
                case ModbusFunction::ReadCoils:
                case ModbusFunction::ReadDiscreteInputs:
                    if (transaction.quantity > ModbusConstants::MAX_COILS_PER_REQUEST)
                    {
                        ESP_LOGE(TAG, "Quantity exceeds maximum allowed coils");
                        return false;
                    }
                    break;
                default:
                    break;
                }

                return true;
            }

            uint16_t ModbusMaster::getNextTransactionId()
            {
                if (xSemaphoreTake(_transactionSemaphore, portMAX_DELAY) == pdTRUE)
                {
                    uint16_t id = _nextTransactionId++;
                    if (_nextTransactionId == 0)
                        _nextTransactionId = 1;
                    xSemaphoreGive(_transactionSemaphore);
                    return id;
                }
                return 0;
            }

            void ModbusMaster::cleanupTimedOutTransactions()
            {
                ESP_LOGD(TAG,"Cleaning timed out transactions");
                std::lock_guard<std::mutex> lock(_transactionMutex);
                auto now = xTaskGetTickCount();

                for (auto it = _pendingTransactions.begin(); it != _pendingTransactions.end();)
                {
                    if ((now - it->second.timestamp) > pdMS_TO_TICKS(_config.responseTimeoutMs))
                    {
                        if (it->second.callback)
                        {
                            it->second.callback(it->second.slaveAddr, it->second.function, {}, TransactionStatus::Timeout);
                        }
                        ESP_LOGD(TAG, "Transaction %u to slave %d timed out", it->first, it->second.slaveAddr);
                        _statistics.timeouts++;
                        _statistics.failedTransactions++;
                        it = _pendingTransactions.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }
            }

            bool ModbusMaster::retryTransaction(ModbusTransaction &transaction)
            {
                if (transaction.retryCount >= ModbusConstants::MAX_RETRIES)
                {
                    return false;
                }

                transaction.retryCount++;
                _statistics.retries++;
                transaction.timestamp = xTaskGetTickCount();

                return xQueueSendToFront(_requestQueue, &transaction, 0) == pdTRUE;
            }

            void ModbusMaster::updateStatistics(const ModbusTransaction &transaction)
            {
                switch (transaction.status)
                {
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

            size_t ModbusMaster::calculateExpectedResponseLength(const ModbusTransaction &transaction) const
            {
                // Delegate to the packager to calculate the expected response length
                return _packager->calculateExpectedResponseLength(transaction);
            }

            bool ModbusMaster::handleModbusException(uint8_t exceptionCode, ModbusTransaction &transaction)
            {
                ModbusError error = static_cast<ModbusError>(exceptionCode);
                const char *errorStr;

                switch (error)
                {
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

                ESP_LOGW(TAG, "Modbus exception received from slave %" PRIu8 ": %s (0x%02" PRIX8 ")",
                         transaction.slaveAddr, errorStr, exceptionCode);

                transaction.status = TransactionStatus::ExceptionReceived;
                return false;
            }

            uint16_t ModbusMaster::calculateCRC(const uint8_t *data, size_t length) const
            {
                // Use the shared CRC calculation utility
                return utils::calculateCRC(data, length);
            }

            void ModbusMaster::setPackager(std::shared_ptr<ModbusPackager> packager)
            {
                if (!packager) {
                    ESP_LOGE(TAG, "Cannot set null packager");
                    return;
                }
                
                _packager = packager;
                ESP_LOGI(TAG, "Packager changed");
            }

            void ModbusMaster::initializeDefaultPackager()
            {
                // If a packager was provided in the config, use it
                if (_config.packager) {
                    ESP_LOGI(TAG, "Using packager from config");
                    _packager = _config.packager;
                    return;
                }
                
                // Otherwise, create a default packager based on transport type
                // We'll check if this is a TCP transport by checking for a specific header size
                bool isTcpTransport = (_transport->getHeaderSize() == ModbusConstants::TCP_HEADER_SIZE);
                
                _packager = ModbusPackagerFactory::createDefaultPackager(!isTcpTransport);
                ESP_LOGI(TAG, "Created default %s packager", isTcpTransport ? "MBAP" : "RTU");
            }

        } // namespace modbus
    } // namespace net
} // namespace speed
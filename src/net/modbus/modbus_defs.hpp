#pragma once

#include <cstdint>
#include <vector>

namespace speed {
namespace net {
namespace modbus {

// Standard Modbus function codes
enum class ModbusFunction : uint8_t {
    ReadCoils = 0x01,
    ReadDiscreteInputs = 0x02,
    ReadHoldingRegisters = 0x03,
    ReadInputRegisters = 0x04,
    WriteSingleCoil = 0x05,
    WriteSingleRegister = 0x06,
    WriteMultipleCoils = 0x0F,
    WriteMultipleRegisters = 0x10,
    ReadWriteMultipleRegisters = 0x17,
    Diagnostics = 0x08
};

// Modbus exception codes
enum class ModbusError : uint8_t {
    NoError = 0x00,
    IllegalFunction = 0x01,
    IllegalDataAddress = 0x02,
    IllegalDataValue = 0x03,
    SlaveDeviceFailure = 0x04,
    Acknowledge = 0x05,
    SlaveDeviceBusy = 0x06,
    MemoryParityError = 0x08,
    GatewayPathUnavailable = 0x0A,
    GatewayTargetFailedToRespond = 0x0B
};

// Modbus protocol constants
struct ModbusConstants {
    static constexpr size_t MAX_ADU_LENGTH = 256;
    static constexpr size_t TCP_HEADER_SIZE = 7;  // MBAP header (7 bytes)
    static constexpr size_t RTU_HEADER_SIZE = 1;  // Address (1 byte)
    static constexpr size_t RTU_CRC_SIZE = 2;     // CRC (2 bytes)
    static constexpr size_t MAX_PDU_LENGTH = 253;  // Function (1) + Data (252)
    
    // Response lengths
    static constexpr size_t TCP_EXCEPTION_LENGTH = TCP_HEADER_SIZE + 2;  // MBAP + Function + Exception
    static constexpr size_t RTU_EXCEPTION_LENGTH = RTU_HEADER_SIZE + 2 + RTU_CRC_SIZE;  // Addr + Function + Exception + CRC
    
    // Function codes
    static constexpr uint8_t READ_COILS = 0x01;
    static constexpr uint8_t READ_DISCRETE_INPUTS = 0x02;
    static constexpr uint8_t READ_HOLDING_REGISTERS = 0x03;
    static constexpr uint8_t READ_INPUT_REGISTERS = 0x04;
    static constexpr uint8_t WRITE_SINGLE_COIL = 0x05;
    static constexpr uint8_t WRITE_SINGLE_REGISTER = 0x06;
    static constexpr uint8_t WRITE_MULTIPLE_COILS = 0x0F;
    static constexpr uint8_t WRITE_MULTIPLE_REGISTERS = 0x10;
    static constexpr uint8_t READ_WRITE_MULTIPLE_REGISTERS = 0x17;
    static constexpr uint8_t DIAGNOSTICS = 0x08;
    
    // Exception codes
    static constexpr uint8_t ILLEGAL_FUNCTION = 0x01;
    static constexpr uint8_t ILLEGAL_DATA_ADDRESS = 0x02;
    static constexpr uint8_t ILLEGAL_DATA_VALUE = 0x03;
    static constexpr uint8_t SLAVE_DEVICE_FAILURE = 0x04;
    static constexpr uint8_t ACKNOWLEDGE = 0x05;
    static constexpr uint8_t SLAVE_DEVICE_BUSY = 0x06;
    static constexpr uint8_t NEGATIVE_ACKNOWLEDGE = 0x07;
    static constexpr uint8_t MEMORY_PARITY_ERROR = 0x08;
    
    // Protocol IDs
    static constexpr uint16_t MODBUS_PROTOCOL_ID = 0x0000;  // Standard Modbus protocol
    
    // Default values
    static constexpr uint16_t DEFAULT_TCP_PORT = 502;
    static constexpr uint32_t DEFAULT_BAUD_RATE = 9600;
    static constexpr uint16_t DEFAULT_RESPONSE_TIMEOUT = 2000;  // 2 seconds
    static constexpr uint8_t DEFAULT_RETRIES = 3;
    static constexpr uint16_t DEFAULT_INTER_FRAME_DELAY = 4;  // 4ms (> 3.5 char at 9600)
};

// Transaction status
enum class TransactionStatus {
    Success,
    Timeout,
    CrcError,
    ExceptionReceived,
    ConnectionError,
    InvalidResponse,
    Pending
};

// Statistics counters
struct ModbusStatistics {
    uint32_t messagesReceived{0};
    uint32_t messagesSent{0};
    uint32_t successfulTransactions{0};
    uint32_t failedTransactions{0};
    uint32_t timeouts{0};
    uint32_t crcErrors{0};
    uint32_t exceptionResponses{0};
    uint32_t retries{0};
    
    void reset() {
        messagesReceived = 0;
        messagesSent = 0;
        successfulTransactions = 0;
        failedTransactions = 0;
        timeouts = 0;
        crcErrors = 0;
        exceptionResponses = 0;
        retries = 0;
    }
};

// Transaction information
struct ModbusTransaction {
    uint16_t transactionId;
    uint8_t slaveAddr;
    ModbusFunction function;
    uint16_t startAddress;
    uint16_t quantity;
    std::vector<uint8_t> data;
    TransactionStatus status;
    uint8_t retryCount;
    uint32_t timestamp;
};

} // namespace modbus
} // namespace net
} // namespace speed
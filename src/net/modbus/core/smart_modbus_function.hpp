#pragma once

#include <SmartEnumCpp/SmartEnum.hpp>
#include <string>

namespace speed
{
    namespace net
    {
        namespace modbus
        {
            /**
             * @brief Polymorphic SmartEnum implementation of ModbusFunction
             * 
             * Provides enhanced functionality for Modbus function codes, including
             * the ability to get descriptions, check properties, and simplify switch-case logic.
             */
            class SmartModbusFunction : public SmartEnum<SmartModbusFunction, uint8_t>
            {
            public:
                // Get the function code name
                virtual std::string getDescription() const { return Name(); }
                
                // Get whether this function is a read function
                virtual bool isReadFunction() const { return false; }
                
                // Get whether this function is a write function
                virtual bool isWriteFunction() const { return false; }
                
                // Get whether this function operates on coils
                virtual bool operatesOnCoils() const { return false; }
                
                // Get whether this function operates on registers
                virtual bool operatesOnRegisters() const { return false; }
                
                // Calculate expected response length for this function
                virtual size_t calculateExpectedResponseLength(uint16_t quantity) const = 0;
                
                // Standard Modbus function code instances
                static const SmartModbusFunction& ReadCoils;
                static const SmartModbusFunction& ReadDiscreteInputs;
                static const SmartModbusFunction& ReadHoldingRegisters;
                static const SmartModbusFunction& ReadInputRegisters;
                static const SmartModbusFunction& WriteSingleCoil;
                static const SmartModbusFunction& WriteSingleRegister;
                static const SmartModbusFunction& WriteMultipleCoils;
                static const SmartModbusFunction& WriteMultipleRegisters;
                static const SmartModbusFunction& ReadWriteMultipleRegisters;
                static const SmartModbusFunction& Diagnostics;
                
            protected:
                // Protected constructor for derived classes
                SmartModbusFunction(const std::string& name, uint8_t value) : SmartEnum(name, value) {}
            };

            /**
             * @brief ReadCoils function implementation (0x01)
             */
            class ReadCoilsFunction : public SmartModbusFunction
            {
            public:
                ReadCoilsFunction() : SmartModbusFunction("ReadCoils", 0x01) {}
                
                std::string getDescription() const override { 
                    return "Read Coils (0x01)"; 
                }
                
                bool isReadFunction() const override { return true; }
                bool operatesOnCoils() const override { return true; }
                
                size_t calculateExpectedResponseLength(uint16_t quantity) const override {
                    // Calculate number of bytes needed to hold all coils (8 coils per byte)
                    size_t byteCount = (quantity + 7) / 8;
                    // Response format: slave addr (1) + function code (1) + byte count (1) + data (byteCount) + CRC (2)
                    return 3 + byteCount + 2;
                }
            };

            /**
             * @brief ReadDiscreteInputs function implementation (0x02)
             */
            class ReadDiscreteInputsFunction : public SmartModbusFunction
            {
            public:
                ReadDiscreteInputsFunction() : SmartModbusFunction("ReadDiscreteInputs", 0x02) {}
                
                std::string getDescription() const override { 
                    return "Read Discrete Inputs (0x02)"; 
                }
                
                bool isReadFunction() const override { return true; }
                bool operatesOnCoils() const override { return true; }
                
                size_t calculateExpectedResponseLength(uint16_t quantity) const override {
                    // Calculate number of bytes needed to hold all inputs (8 inputs per byte)
                    size_t byteCount = (quantity + 7) / 8;
                    // Response format: slave addr (1) + function code (1) + byte count (1) + data (byteCount) + CRC (2)
                    return 3 + byteCount + 2;
                }
            };

            /**
             * @brief ReadHoldingRegisters function implementation (0x03)
             */
            class ReadHoldingRegistersFunction : public SmartModbusFunction
            {
            public:
                ReadHoldingRegistersFunction() : SmartModbusFunction("ReadHoldingRegisters", 0x03) {}
                
                std::string getDescription() const override { 
                    return "Read Holding Registers (0x03)"; 
                }
                
                bool isReadFunction() const override { return true; }
                bool operatesOnRegisters() const override { return true; }
                
                size_t calculateExpectedResponseLength(uint16_t quantity) const override {
                    // Response format: slave addr (1) + function code (1) + byte count (1) + data (2*quantity) + CRC (2)
                    return 3 + (quantity * 2) + 2;
                }
            };

            /**
             * @brief ReadInputRegisters function implementation (0x04)
             */
            class ReadInputRegistersFunction : public SmartModbusFunction
            {
            public:
                ReadInputRegistersFunction() : SmartModbusFunction("ReadInputRegisters", 0x04) {}
                
                std::string getDescription() const override { 
                    return "Read Input Registers (0x04)"; 
                }
                
                bool isReadFunction() const override { return true; }
                bool operatesOnRegisters() const override { return true; }
                
                size_t calculateExpectedResponseLength(uint16_t quantity) const override {
                    // Response format: slave addr (1) + function code (1) + byte count (1) + data (2*quantity) + CRC (2)
                    return 3 + (quantity * 2) + 2;
                }
            };

            /**
             * @brief WriteSingleCoil function implementation (0x05)
             */
            class WriteSingleCoilFunction : public SmartModbusFunction
            {
            public:
                WriteSingleCoilFunction() : SmartModbusFunction("WriteSingleCoil", 0x05) {}
                
                std::string getDescription() const override { 
                    return "Write Single Coil (0x05)"; 
                }
                
                bool isWriteFunction() const override { return true; }
                bool operatesOnCoils() const override { return true; }
                
                size_t calculateExpectedResponseLength(uint16_t quantity) const override {
                    // Response format: slave addr (1) + function code (1) + address (2) + value (2) + CRC (2)
                    return 8;
                }
            };

            /**
             * @brief WriteSingleRegister function implementation (0x06)
             */
            class WriteSingleRegisterFunction : public SmartModbusFunction
            {
            public:
                WriteSingleRegisterFunction() : SmartModbusFunction("WriteSingleRegister", 0x06) {}
                
                std::string getDescription() const override { 
                    return "Write Single Register (0x06)"; 
                }
                
                bool isWriteFunction() const override { return true; }
                bool operatesOnRegisters() const override { return true; }
                
                size_t calculateExpectedResponseLength(uint16_t quantity) const override {
                    // Response format: slave addr (1) + function code (1) + address (2) + value (2) + CRC (2)
                    return 8;
                }
            };

            /**
             * @brief WriteMultipleCoils function implementation (0x0F)
             */
            class WriteMultipleCoilsFunction : public SmartModbusFunction
            {
            public:
                WriteMultipleCoilsFunction() : SmartModbusFunction("WriteMultipleCoils", 0x0F) {}
                
                std::string getDescription() const override { 
                    return "Write Multiple Coils (0x0F)"; 
                }
                
                bool isWriteFunction() const override { return true; }
                bool operatesOnCoils() const override { return true; }
                
                size_t calculateExpectedResponseLength(uint16_t quantity) const override {
                    // Response format: slave addr (1) + function code (1) + address (2) + quantity (2) + CRC (2)
                    return 8;
                }
            };

            /**
             * @brief WriteMultipleRegisters function implementation (0x10)
             */
            class WriteMultipleRegistersFunction : public SmartModbusFunction
            {
            public:
                WriteMultipleRegistersFunction() : SmartModbusFunction("WriteMultipleRegisters", 0x10) {}
                
                std::string getDescription() const override { 
                    return "Write Multiple Registers (0x10)"; 
                }
                
                bool isWriteFunction() const override { return true; }
                bool operatesOnRegisters() const override { return true; }
                
                size_t calculateExpectedResponseLength(uint16_t quantity) const override {
                    // Response format: slave addr (1) + function code (1) + address (2) + quantity (2) + CRC (2)
                    return 8;
                }
            };

            /**
             * @brief ReadWriteMultipleRegisters function implementation (0x17)
             */
            class ReadWriteMultipleRegistersFunction : public SmartModbusFunction
            {
            public:
                ReadWriteMultipleRegistersFunction() : SmartModbusFunction("ReadWriteMultipleRegisters", 0x17) {}
                
                std::string getDescription() const override { 
                    return "Read/Write Multiple Registers (0x17)"; 
                }
                
                bool isReadFunction() const override { return true; }
                bool isWriteFunction() const override { return true; }
                bool operatesOnRegisters() const override { return true; }
                
                size_t calculateExpectedResponseLength(uint16_t quantity) const override {
                    // Response format: slave addr (1) + function code (1) + byte count (1) + data (2*readQuantity) + CRC (2)
                    // Note: quantity here represents the read quantity
                    return 3 + (quantity * 2) + 2;
                }
            };

            /**
             * @brief Diagnostics function implementation (0x08)
             */
            class DiagnosticsFunction : public SmartModbusFunction
            {
            public:
                DiagnosticsFunction() : SmartModbusFunction("Diagnostics", 0x08) {}
                
                std::string getDescription() const override { 
                    return "Diagnostics (0x08)"; 
                }
                
                size_t calculateExpectedResponseLength(uint16_t quantity) const override {
                    // Response format depends on sub-function, default response is:
                    // slave addr (1) + function code (1) + sub-function (2) + data (2) + CRC (2)
                    return 8;
                }
            };

            // Initialize the static instances
            inline const SmartModbusFunction& SmartModbusFunction::ReadCoils = ReadCoilsFunction();
            inline const SmartModbusFunction& SmartModbusFunction::ReadDiscreteInputs = ReadDiscreteInputsFunction();
            inline const SmartModbusFunction& SmartModbusFunction::ReadHoldingRegisters = ReadHoldingRegistersFunction();
            inline const SmartModbusFunction& SmartModbusFunction::ReadInputRegisters = ReadInputRegistersFunction();
            inline const SmartModbusFunction& SmartModbusFunction::WriteSingleCoil = WriteSingleCoilFunction();
            inline const SmartModbusFunction& SmartModbusFunction::WriteSingleRegister = WriteSingleRegisterFunction();
            inline const SmartModbusFunction& SmartModbusFunction::WriteMultipleCoils = WriteMultipleCoilsFunction();
            inline const SmartModbusFunction& SmartModbusFunction::WriteMultipleRegisters = WriteMultipleRegistersFunction();
            inline const SmartModbusFunction& SmartModbusFunction::ReadWriteMultipleRegisters = ReadWriteMultipleRegistersFunction();
            inline const SmartModbusFunction& SmartModbusFunction::Diagnostics = DiagnosticsFunction();
            
            // Compatibility type alias to allow gradual migration
            using ModbusFunctionSmart = SmartModbusFunction;

        } // namespace modbus
    } // namespace net
} // namespace speed
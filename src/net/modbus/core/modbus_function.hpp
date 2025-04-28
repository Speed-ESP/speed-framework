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
            class ModbusFunction : public SmartEnum<ModbusFunction, uint8_t>
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
                static const ModbusFunction& ReadCoils;
                static const ModbusFunction& ReadDiscreteInputs;
                static const ModbusFunction& ReadHoldingRegisters;
                static const ModbusFunction& ReadInputRegisters;
                static const ModbusFunction& WriteSingleCoil;
                static const ModbusFunction& WriteSingleRegister;
                static const ModbusFunction& WriteMultipleCoils;
                static const ModbusFunction& WriteMultipleRegisters;
                static const ModbusFunction& ReadWriteMultipleRegisters;
                static const ModbusFunction& Diagnostics;
                
            protected:
                // Protected constructor for derived classes
                ModbusFunction(const std::string& name, uint8_t value) : SmartEnum(name, value) {}
            };

            /**
             * @brief ReadCoils function implementation (0x01)
             */
            class ReadCoilsFunction : public ModbusFunction
            {
            public:
                ReadCoilsFunction() : ModbusFunction("ReadCoils", 0x01) {}
                
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
            class ReadDiscreteInputsFunction : public ModbusFunction
            {
            public:
                ReadDiscreteInputsFunction() : ModbusFunction("ReadDiscreteInputs", 0x02) {}
                
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
            class ReadHoldingRegistersFunction : public ModbusFunction
            {
            public:
                ReadHoldingRegistersFunction() : ModbusFunction("ReadHoldingRegisters", 0x03) {}
                
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
            class ReadInputRegistersFunction : public ModbusFunction
            {
            public:
                ReadInputRegistersFunction() : ModbusFunction("ReadInputRegisters", 0x04) {}
                
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
            class WriteSingleCoilFunction : public ModbusFunction
            {
            public:
                WriteSingleCoilFunction() : ModbusFunction("WriteSingleCoil", 0x05) {}
                
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
            class WriteSingleRegisterFunction : public ModbusFunction
            {
            public:
                WriteSingleRegisterFunction() : ModbusFunction("WriteSingleRegister", 0x06) {}
                
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
            class WriteMultipleCoilsFunction : public ModbusFunction
            {
            public:
                WriteMultipleCoilsFunction() : ModbusFunction("WriteMultipleCoils", 0x0F) {}
                
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
            class WriteMultipleRegistersFunction : public ModbusFunction
            {
            public:
                WriteMultipleRegistersFunction() : ModbusFunction("WriteMultipleRegisters", 0x10) {}
                
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
            class ReadWriteMultipleRegistersFunction : public ModbusFunction
            {
            public:
                ReadWriteMultipleRegistersFunction() : ModbusFunction("ReadWriteMultipleRegisters", 0x17) {}
                
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
            class DiagnosticsFunction : public ModbusFunction
            {
            public:
                DiagnosticsFunction() : ModbusFunction("Diagnostics", 0x08) {}
                
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
            inline const ModbusFunction& ModbusFunction::ReadCoils = ReadCoilsFunction();
            inline const ModbusFunction& ModbusFunction::ReadDiscreteInputs = ReadDiscreteInputsFunction();
            inline const ModbusFunction& ModbusFunction::ReadHoldingRegisters = ReadHoldingRegistersFunction();
            inline const ModbusFunction& ModbusFunction::ReadInputRegisters = ReadInputRegistersFunction();
            inline const ModbusFunction& ModbusFunction::WriteSingleCoil = WriteSingleCoilFunction();
            inline const ModbusFunction& ModbusFunction::WriteSingleRegister = WriteSingleRegisterFunction();
            inline const ModbusFunction& ModbusFunction::WriteMultipleCoils = WriteMultipleCoilsFunction();
            inline const ModbusFunction& ModbusFunction::WriteMultipleRegisters = WriteMultipleRegistersFunction();
            inline const ModbusFunction& ModbusFunction::ReadWriteMultipleRegisters = ReadWriteMultipleRegistersFunction();
            inline const ModbusFunction& ModbusFunction::Diagnostics = DiagnosticsFunction();

        } // namespace modbus
    } // namespace net
} // namespace speed
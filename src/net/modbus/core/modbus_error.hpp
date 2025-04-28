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
             * @brief Polymorphic SmartEnum implementation of ModbusError
             * 
             * Provides enhanced functionality for Modbus exception codes, including
             * the ability to get descriptions, check error properties, and simplify switch-case logic.
             */
            class ModbusError : public SmartEnum<ModbusError, uint8_t>
            {
            public:
                // Get a human-readable description of the error
                virtual std::string getDescription() const { return Name(); }
                
                // Get a detailed explanation of the error
                virtual std::string getDetailedDescription() const { return getDescription(); }
                
                // Check if the error requires retry
                virtual bool shouldRetry() const { return false; }
                
                // Get recommended delay before retry (in milliseconds)
                virtual uint32_t getRecommendedRetryDelay() const { return 0; }
                
                // Check if the error is recoverable
                virtual bool isRecoverable() const { return true; }
                
                // Error instances
                static const ModbusError& NoError;
                static const ModbusError& IllegalFunction;
                static const ModbusError& IllegalDataAddress;
                static const ModbusError& IllegalDataValue;
                static const ModbusError& SlaveDeviceFailure;
                static const ModbusError& Acknowledge;
                static const ModbusError& SlaveDeviceBusy;
                static const ModbusError& MemoryParityError;
                static const ModbusError& GatewayPathUnavailable;
                static const ModbusError& GatewayTargetFailedToRespond;
                
            protected:
                // Protected constructor for derived classes
                ModbusError(const std::string& name, uint8_t value) : SmartEnum(name, value) {}
            };

            /**
             * @brief NoError implementation (0x00)
             */
            class NoErrorImpl : public ModbusError
            {
            public:
                NoErrorImpl() : ModbusError("NoError", 0x00) {}
                
                std::string getDescription() const override { 
                    return "No error"; 
                }
                
                std::string getDetailedDescription() const override {
                    return "The operation completed successfully without errors.";
                }
            };

            /**
             * @brief IllegalFunction implementation (0x01)
             */
            class IllegalFunctionImpl : public ModbusError
            {
            public:
                IllegalFunctionImpl() : ModbusError("IllegalFunction", 0x01) {}
                
                std::string getDescription() const override { 
                    return "Illegal function"; 
                }
                
                std::string getDetailedDescription() const override {
                    return "The function code received in the query is not recognized or allowed by the slave.";
                }
                
                bool isRecoverable() const override { return false; }
            };

            /**
             * @brief IllegalDataAddress implementation (0x02)
             */
            class IllegalDataAddressImpl : public ModbusError
            {
            public:
                IllegalDataAddressImpl() : ModbusError("IllegalDataAddress", 0x02) {}
                
                std::string getDescription() const override { 
                    return "Illegal data address"; 
                }
                
                std::string getDetailedDescription() const override {
                    return "The data address received in the query is not an allowable address for the slave.";
                }
                
                bool isRecoverable() const override { return false; }
            };

            /**
             * @brief IllegalDataValue implementation (0x03)
             */
            class IllegalDataValueImpl : public ModbusError
            {
            public:
                IllegalDataValueImpl() : ModbusError("IllegalDataValue", 0x03) {}
                
                std::string getDescription() const override { 
                    return "Illegal data value"; 
                }
                
                std::string getDetailedDescription() const override {
                    return "A value contained in the query data field is not an allowable value for the slave.";
                }
                
                bool isRecoverable() const override { return false; }
            };

            /**
             * @brief SlaveDeviceFailure implementation (0x04)
             */
            class SlaveDeviceFailureImpl : public ModbusError
            {
            public:
                SlaveDeviceFailureImpl() : ModbusError("SlaveDeviceFailure", 0x04) {}
                
                std::string getDescription() const override { 
                    return "Slave device failure"; 
                }
                
                std::string getDetailedDescription() const override {
                    return "An unrecoverable error occurred while the slave was attempting to perform the requested action.";
                }
                
                bool shouldRetry() const override { return true; }
                uint32_t getRecommendedRetryDelay() const override { return 5000; }
                bool isRecoverable() const override { return false; }
            };

            /**
             * @brief Acknowledge implementation (0x05)
             */
            class AcknowledgeImpl : public ModbusError
            {
            public:
                AcknowledgeImpl() : ModbusError("Acknowledge", 0x05) {}
                
                std::string getDescription() const override { 
                    return "Acknowledge"; 
                }
                
                std::string getDetailedDescription() const override {
                    return "The slave has accepted the request and is processing it, but a long duration of time will be required to do so.";
                }
                
                bool shouldRetry() const override { return true; }
                uint32_t getRecommendedRetryDelay() const override { return 1000; }
            };

            /**
             * @brief SlaveDeviceBusy implementation (0x06)
             */
            class SlaveDeviceBusyImpl : public ModbusError
            {
            public:
                SlaveDeviceBusyImpl() : ModbusError("SlaveDeviceBusy", 0x06) {}
                
                std::string getDescription() const override { 
                    return "Slave device busy"; 
                }
                
                std::string getDetailedDescription() const override {
                    return "The slave is engaged in processing a long-duration program command.";
                }
                
                bool shouldRetry() const override { return true; }
                uint32_t getRecommendedRetryDelay() const override { return 2000; }
            };

            /**
             * @brief MemoryParityError implementation (0x08)
             */
            class MemoryParityErrorImpl : public ModbusError
            {
            public:
                MemoryParityErrorImpl() : ModbusError("MemoryParityError", 0x08) {}
                
                std::string getDescription() const override { 
                    return "Memory parity error"; 
                }
                
                std::string getDetailedDescription() const override {
                    return "The slave attempted to read extended memory, but detected a parity error in the memory.";
                }
                
                bool shouldRetry() const override { return true; }
                uint32_t getRecommendedRetryDelay() const override { return 500; }
            };

            /**
             * @brief GatewayPathUnavailable implementation (0x0A)
             */
            class GatewayPathUnavailableImpl : public ModbusError
            {
            public:
                GatewayPathUnavailableImpl() : ModbusError("GatewayPathUnavailable", 0x0A) {}
                
                std::string getDescription() const override { 
                    return "Gateway path unavailable"; 
                }
                
                std::string getDetailedDescription() const override {
                    return "The gateway was unable to establish a connection path to the target device.";
                }
                
                bool shouldRetry() const override { return true; }
                uint32_t getRecommendedRetryDelay() const override { return 3000; }
            };

            /**
             * @brief GatewayTargetFailedToRespond implementation (0x0B)
             */
            class GatewayTargetFailedToRespondImpl : public ModbusError
            {
            public:
                GatewayTargetFailedToRespondImpl() : ModbusError("GatewayTargetFailedToRespond", 0x0B) {}
                
                std::string getDescription() const override { 
                    return "Gateway target failed to respond"; 
                }
                
                std::string getDetailedDescription() const override {
                    return "No response was obtained from the target device.";
                }
                
                bool shouldRetry() const override { return true; }
                uint32_t getRecommendedRetryDelay() const override { return 3000; }
            };

            // Initialize the static instances
            inline const ModbusError& ModbusError::NoError = NoErrorImpl();
            inline const ModbusError& ModbusError::IllegalFunction = IllegalFunctionImpl();
            inline const ModbusError& ModbusError::IllegalDataAddress = IllegalDataAddressImpl();
            inline const ModbusError& ModbusError::IllegalDataValue = IllegalDataValueImpl();
            inline const ModbusError& ModbusError::SlaveDeviceFailure = SlaveDeviceFailureImpl();
            inline const ModbusError& ModbusError::Acknowledge = AcknowledgeImpl();
            inline const ModbusError& ModbusError::SlaveDeviceBusy = SlaveDeviceBusyImpl();
            inline const ModbusError& ModbusError::MemoryParityError = MemoryParityErrorImpl();
            inline const ModbusError& ModbusError::GatewayPathUnavailable = GatewayPathUnavailableImpl();
            inline const ModbusError& ModbusError::GatewayTargetFailedToRespond = GatewayTargetFailedToRespondImpl();

        } // namespace modbus
    } // namespace net
} // namespace speed
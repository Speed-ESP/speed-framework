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
            class SmartModbusError : public SmartEnum<SmartModbusError, uint8_t>
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
                static const SmartModbusError& NoError;
                static const SmartModbusError& IllegalFunction;
                static const SmartModbusError& IllegalDataAddress;
                static const SmartModbusError& IllegalDataValue;
                static const SmartModbusError& SlaveDeviceFailure;
                static const SmartModbusError& Acknowledge;
                static const SmartModbusError& SlaveDeviceBusy;
                static const SmartModbusError& MemoryParityError;
                static const SmartModbusError& GatewayPathUnavailable;
                static const SmartModbusError& GatewayTargetFailedToRespond;
                
            protected:
                // Protected constructor for derived classes
                SmartModbusError(const std::string& name, uint8_t value) : SmartEnum(name, value) {}
            };

            /**
             * @brief NoError implementation (0x00)
             */
            class NoErrorImpl : public SmartModbusError
            {
            public:
                NoErrorImpl() : SmartModbusError("NoError", 0x00) {}
                
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
            class IllegalFunctionImpl : public SmartModbusError
            {
            public:
                IllegalFunctionImpl() : SmartModbusError("IllegalFunction", 0x01) {}
                
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
            class IllegalDataAddressImpl : public SmartModbusError
            {
            public:
                IllegalDataAddressImpl() : SmartModbusError("IllegalDataAddress", 0x02) {}
                
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
            class IllegalDataValueImpl : public SmartModbusError
            {
            public:
                IllegalDataValueImpl() : SmartModbusError("IllegalDataValue", 0x03) {}
                
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
            class SlaveDeviceFailureImpl : public SmartModbusError
            {
            public:
                SlaveDeviceFailureImpl() : SmartModbusError("SlaveDeviceFailure", 0x04) {}
                
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
            class AcknowledgeImpl : public SmartModbusError
            {
            public:
                AcknowledgeImpl() : SmartModbusError("Acknowledge", 0x05) {}
                
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
            class SlaveDeviceBusyImpl : public SmartModbusError
            {
            public:
                SlaveDeviceBusyImpl() : SmartModbusError("SlaveDeviceBusy", 0x06) {}
                
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
            class MemoryParityErrorImpl : public SmartModbusError
            {
            public:
                MemoryParityErrorImpl() : SmartModbusError("MemoryParityError", 0x08) {}
                
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
            class GatewayPathUnavailableImpl : public SmartModbusError
            {
            public:
                GatewayPathUnavailableImpl() : SmartModbusError("GatewayPathUnavailable", 0x0A) {}
                
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
            class GatewayTargetFailedToRespondImpl : public SmartModbusError
            {
            public:
                GatewayTargetFailedToRespondImpl() : SmartModbusError("GatewayTargetFailedToRespond", 0x0B) {}
                
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
            inline const SmartModbusError& SmartModbusError::NoError = NoErrorImpl();
            inline const SmartModbusError& SmartModbusError::IllegalFunction = IllegalFunctionImpl();
            inline const SmartModbusError& SmartModbusError::IllegalDataAddress = IllegalDataAddressImpl();
            inline const SmartModbusError& SmartModbusError::IllegalDataValue = IllegalDataValueImpl();
            inline const SmartModbusError& SmartModbusError::SlaveDeviceFailure = SlaveDeviceFailureImpl();
            inline const SmartModbusError& SmartModbusError::Acknowledge = AcknowledgeImpl();
            inline const SmartModbusError& SmartModbusError::SlaveDeviceBusy = SlaveDeviceBusyImpl();
            inline const SmartModbusError& SmartModbusError::MemoryParityError = MemoryParityErrorImpl();
            inline const SmartModbusError& SmartModbusError::GatewayPathUnavailable = GatewayPathUnavailableImpl();
            inline const SmartModbusError& SmartModbusError::GatewayTargetFailedToRespond = GatewayTargetFailedToRespondImpl();
            
            // Compatibility type alias to allow gradual migration
            using ModbusErrorSmart = SmartModbusError;

        } // namespace modbus
    } // namespace net
} // namespace speed
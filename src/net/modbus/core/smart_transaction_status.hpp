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
             * @brief Polymorphic SmartEnum implementation of TransactionStatus
             * 
             * Provides enhanced functionality for Modbus transaction status, including
             * the ability to get descriptions, check status properties, and simplify switch-case logic.
             */
            class SmartTransactionStatus : public SmartEnum<SmartTransactionStatus, uint8_t>
            {
            public:
                // Get a human-readable description of the status
                virtual std::string getDescription() const { return Name(); }
                
                // Check if the transaction was successful
                virtual bool isSuccessful() const { return false; }
                
                // Check if the transaction is pending completion
                virtual bool isPending() const { return false; }
                
                // Check if the transaction failed
                virtual bool isFailed() const { return false; }
                
                // Check if the transaction failed due to timeout
                virtual bool isTimeout() const { return false; }
                
                // Check if the transaction failed due to connection error
                virtual bool isConnectionError() const { return false; }
                
                // Status instances
                static const SmartTransactionStatus& Success;
                static const SmartTransactionStatus& Timeout;
                static const SmartTransactionStatus& CrcError;
                static const SmartTransactionStatus& ExceptionReceived;
                static const SmartTransactionStatus& ConnectionError;
                static const SmartTransactionStatus& InvalidResponse;
                static const SmartTransactionStatus& Pending;
                
            protected:
                // Protected constructor for derived classes
                SmartTransactionStatus(const std::string& name, uint8_t value) : SmartEnum(name, value) {}
            };

            /**
             * @brief Success status implementation
             */
            class SuccessStatus : public SmartTransactionStatus
            {
            public:
                SuccessStatus() : SmartTransactionStatus("Success", 0) {}
                
                std::string getDescription() const override { 
                    return "Transaction completed successfully"; 
                }
                
                bool isSuccessful() const override { return true; }
            };

            /**
             * @brief Timeout status implementation
             */
            class TimeoutStatus : public SmartTransactionStatus
            {
            public:
                TimeoutStatus() : SmartTransactionStatus("Timeout", 1) {}
                
                std::string getDescription() const override { 
                    return "Transaction timed out waiting for response"; 
                }
                
                bool isFailed() const override { return true; }
                bool isTimeout() const override { return true; }
            };

            /**
             * @brief CrcError status implementation
             */
            class CrcErrorStatus : public SmartTransactionStatus
            {
            public:
                CrcErrorStatus() : SmartTransactionStatus("CrcError", 2) {}
                
                std::string getDescription() const override { 
                    return "CRC error in response"; 
                }
                
                bool isFailed() const override { return true; }
            };

            /**
             * @brief ExceptionReceived status implementation
             */
            class ExceptionReceivedStatus : public SmartTransactionStatus
            {
            public:
                ExceptionReceivedStatus() : SmartTransactionStatus("ExceptionReceived", 3) {}
                
                std::string getDescription() const override { 
                    return "Modbus exception response received"; 
                }
                
                bool isFailed() const override { return true; }
            };

            /**
             * @brief ConnectionError status implementation
             */
            class ConnectionErrorStatus : public SmartTransactionStatus
            {
            public:
                ConnectionErrorStatus() : SmartTransactionStatus("ConnectionError", 4) {}
                
                std::string getDescription() const override { 
                    return "Connection error during transaction"; 
                }
                
                bool isFailed() const override { return true; }
                bool isConnectionError() const override { return true; }
            };

            /**
             * @brief InvalidResponse status implementation
             */
            class InvalidResponseStatus : public SmartTransactionStatus
            {
            public:
                InvalidResponseStatus() : SmartTransactionStatus("InvalidResponse", 5) {}
                
                std::string getDescription() const override { 
                    return "Invalid response received"; 
                }
                
                bool isFailed() const override { return true; }
            };

            /**
             * @brief Pending status implementation
             */
            class PendingStatus : public SmartTransactionStatus
            {
            public:
                PendingStatus() : SmartTransactionStatus("Pending", 6) {}
                
                std::string getDescription() const override { 
                    return "Transaction is pending completion"; 
                }
                
                bool isPending() const override { return true; }
            };

            // Initialize the static instances
            inline const SmartTransactionStatus& SmartTransactionStatus::Success = SuccessStatus();
            inline const SmartTransactionStatus& SmartTransactionStatus::Timeout = TimeoutStatus();
            inline const SmartTransactionStatus& SmartTransactionStatus::CrcError = CrcErrorStatus();
            inline const SmartTransactionStatus& SmartTransactionStatus::ExceptionReceived = ExceptionReceivedStatus();
            inline const SmartTransactionStatus& SmartTransactionStatus::ConnectionError = ConnectionErrorStatus();
            inline const SmartTransactionStatus& SmartTransactionStatus::InvalidResponse = InvalidResponseStatus();
            inline const SmartTransactionStatus& SmartTransactionStatus::Pending = PendingStatus();
            
            // Compatibility type alias to allow gradual migration
            using TransactionStatusSmart = SmartTransactionStatus;

        } // namespace modbus
    } // namespace net
} // namespace speed
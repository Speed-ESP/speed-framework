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
            class TransactionStatus : public SmartEnum<TransactionStatus, uint8_t>
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
                static const TransactionStatus& Success;
                static const TransactionStatus& Timeout;
                static const TransactionStatus& CrcError;
                static const TransactionStatus& ExceptionReceived;
                static const TransactionStatus& ConnectionError;
                static const TransactionStatus& InvalidResponse;
                static const TransactionStatus& Pending;
                
            protected:
                // Protected constructor for derived classes
                TransactionStatus(const std::string& name, uint8_t value) : SmartEnum(name, value) {}
            };

            /**
             * @brief Success status implementation
             */
            class SuccessStatus : public TransactionStatus
            {
            public:
                SuccessStatus() : TransactionStatus("Success", 0) {}
                
                std::string getDescription() const override { 
                    return "Transaction completed successfully"; 
                }
                
                bool isSuccessful() const override { return true; }
            };

            /**
             * @brief Timeout status implementation
             */
            class TimeoutStatus : public TransactionStatus
            {
            public:
                TimeoutStatus() : TransactionStatus("Timeout", 1) {}
                
                std::string getDescription() const override { 
                    return "Transaction timed out waiting for response"; 
                }
                
                bool isFailed() const override { return true; }
                bool isTimeout() const override { return true; }
            };

            /**
             * @brief CrcError status implementation
             */
            class CrcErrorStatus : public TransactionStatus
            {
            public:
                CrcErrorStatus() : TransactionStatus("CrcError", 2) {}
                
                std::string getDescription() const override { 
                    return "CRC error in response"; 
                }
                
                bool isFailed() const override { return true; }
            };

            /**
             * @brief ExceptionReceived status implementation
             */
            class ExceptionReceivedStatus : public TransactionStatus
            {
            public:
                ExceptionReceivedStatus() : TransactionStatus("ExceptionReceived", 3) {}
                
                std::string getDescription() const override { 
                    return "Modbus exception response received"; 
                }
                
                bool isFailed() const override { return true; }
            };

            /**
             * @brief ConnectionError status implementation
             */
            class ConnectionErrorStatus : public TransactionStatus
            {
            public:
                ConnectionErrorStatus() : TransactionStatus("ConnectionError", 4) {}
                
                std::string getDescription() const override { 
                    return "Connection error during transaction"; 
                }
                
                bool isFailed() const override { return true; }
                bool isConnectionError() const override { return true; }
            };

            /**
             * @brief InvalidResponse status implementation
             */
            class InvalidResponseStatus : public TransactionStatus
            {
            public:
                InvalidResponseStatus() : TransactionStatus("InvalidResponse", 5) {}
                
                std::string getDescription() const override { 
                    return "Invalid response received"; 
                }
                
                bool isFailed() const override { return true; }
            };

            /**
             * @brief Pending status implementation
             */
            class PendingStatus : public TransactionStatus
            {
            public:
                PendingStatus() : TransactionStatus("Pending", 6) {}
                
                std::string getDescription() const override { 
                    return "Transaction is pending completion"; 
                }
                
                bool isPending() const override { return true; }
            };

            // Initialize the static instances
            inline const TransactionStatus& TransactionStatus::Success = SuccessStatus();
            inline const TransactionStatus& TransactionStatus::Timeout = TimeoutStatus();
            inline const TransactionStatus& TransactionStatus::CrcError = CrcErrorStatus();
            inline const TransactionStatus& TransactionStatus::ExceptionReceived = ExceptionReceivedStatus();
            inline const TransactionStatus& TransactionStatus::ConnectionError = ConnectionErrorStatus();
            inline const TransactionStatus& TransactionStatus::InvalidResponse = InvalidResponseStatus();
            inline const TransactionStatus& TransactionStatus::Pending = PendingStatus();

        } // namespace modbus
    } // namespace net
} // namespace speed
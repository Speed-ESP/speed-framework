#pragma once

#ifndef __SINGLETON_SERVICE_H__
#define __SINGLETON_SERVICE_H__
#include <mutex>
#include <string>
#include "esp_log.h"

namespace speed::core
{
    /**
     * @brief Base class for implementing thread-safe singletons in the Speed Framework
     * 
     * This template class provides a thread-safe singleton implementation with automatic
     * initialization. It uses std::call_once and mutex locking to ensure thread safety.
     * 
     * Usage example:
     * ```cpp
     * class MyService : public SingletonService<MyService> {
     * protected:
     *     void init() override {
     *         // Initialize your service
     *     }
     * 
     * public:
     *     void doSomething() {
     *         // Service functionality
     *     }
     * };
     * 
     * // Get the singleton instance
     * MyService& service = MyService::get();
     * ```
     * 
     * @tparam TService The derived class type implementing the singleton
     */
    template <class TService>
    class SingletonService
    {
    private:
        inline static TService *_instance = nullptr;
        static inline std::once_flag initFlag;
        inline static std::mutex _init_mutex;  // Mutex for thread-safe initialization
        inline static bool _initialized = false;  // Flag to track initialization state
        inline static const char* TAG = "SingletonService";
    
    private:
        /**
         * @brief Creates and initializes the singleton instance
         * 
         * This method is called exactly once by std::call_once to ensure
         * that initialization happens only once in a thread-safe manner.
         */
        inline static void initialize()
        {
            // Call init within a lock to ensure thread safety
            std::lock_guard<std::mutex> lock(_init_mutex);
            if (!_initialized) {
                 _instance = new TService;
                ((SingletonService *)_instance)->init();
                _initialized = true;
            }
        }

    protected:
        /**
         * @brief Default constructor, only accessible to derived classes
         */
        SingletonService() = default;
        
        /**
         * @brief Initialization method to be overridden by derived classes
         * 
         * This method is called once during the first access to the singleton.
         * Derived classes should override this to perform their initialization.
         */
        virtual void init() {}
        
        /**
         * @brief Check if the service is initialized
         * 
         * @return bool True if the service has been initialized
         */
        bool isInitialized() const { return _initialized; }

    public:
        /**
         * @brief Get the singleton instance
         * 
         * Thread-safe method to get the singleton instance. On first call,
         * the instance is created and initialized. Subsequent calls return
         * the existing instance.
         * 
         * @return TService& Reference to the singleton instance
         */
        inline static TService &get()
        {
            std::call_once(initFlag, &SingletonService::initialize);
            return *_instance;
        }
        
        /**
         * @brief Execute a function with thread-safe initialization protection
         * 
         * Use this method to safely execute code that requires thread safety.
         * The provided function will be executed while holding the initialization lock.
         * 
         * @tparam Func The type of function to execute
         * @param func The function to execute
         */
        template<typename Func>
        static void withInit(Func func) {
            std::lock_guard<std::mutex> lock(_init_mutex);
            func();
        }
    };
}; // namespace speed::core

#endif // __SINGLETON_SERVICE_H__
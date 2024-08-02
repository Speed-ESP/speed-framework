#pragma once

#ifndef __SINGLETON_SERVICE_H__
#define __SINGLETON_SERVICE_H__
#include <mutex>
#include <string>

namespace Speed::Core
{

    template <class TService>
    class SingletonService
    {
    private:
        inline static TService *_instance = nullptr;
        static inline std::once_flag initFlag;

    private:
        inline static void initialize()
        {
            _instance = new TService;
            ((SingletonService *)_instance)->init();
        }

    protected:
        SingletonService() = default;
        virtual void init() {}

    public:
        inline static TService &get()
        {
            std::call_once(initFlag, &SingletonService::initialize);
            return *_instance;
        }
    };
}; // namespace Speed::Core

#endif // __SINGLETON_SERVICE_H__
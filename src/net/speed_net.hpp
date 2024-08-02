#pragma once

#ifndef __SPEED_NET_H__
#define __SPEED_NET_H__
#include <esp_netif.h>
#include "esp_log.h"
#include <core/singleton_service.hpp>
namespace Speed::Net
{
    using namespace Speed::Core;
    class SpeedNet : SingletonService<SpeedNet>
    {
    private:
        inline static bool _initialized = false;

    protected:
        void init() override
        {
            ESP_ERROR_CHECK(esp_netif_init());
            _initialized = true;
        }

    public:
        static SpeedNed &setup()
        {
            return get();
        }
    };

    class SpeedWifi
    {
    private:
        inline static bool _initialized = false;

    public:
        inline static SpeedWifi setup()
        {
            if (_initialized)
            {
                return;
            }
            _initialized = true;
            SpeedNet::setup();
        }
    };
};

#endif // __SPEED_NET_H__
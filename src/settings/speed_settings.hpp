#pragma once

#ifndef __SPEED_SETTINGS_H__
#define __SPEED_SETTINGS_H__
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "esp_mac.h"
#include "esp_log.h"

#include "nvs_flash.h"
#include "nvs.h"
#include "nvs_handle.hpp"

#include <core/singleton_service.hpp>
namespace Speed::Settings
{
    using namespace Speed::Core;
    constexpr const char *TAG_SETTINGS = "SPEED-SETTINGS";
    class SpeedSettings : SingletonService<SpeedSettings>
    {
    private:
        std::string_view ns_name;
        inline static bool initiated = false;

    protected:
        void init() override
        {
            ESP_LOGI(TAG_SETTINGS, "Initializing NVS");
            initiated = true;
            esp_err_t err = nvs_flash_init();
            if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
            {
                ESP_LOGW(TAG_SETTINGS, "VS partition was truncated and needs to be erased");
                ESP_ERROR_CHECK(nvs_flash_erase());
                ESP_LOGW(TAG_SETTINGS, "Retry nvs_flash_init");
                err = nvs_flash_init();
            }
            ESP_ERROR_CHECK(err);
            ESP_LOGI(TAG_SETTINGS, "Settings properly configured");
        }

    public:
        static SpeedSettings &setup(std::string_view ns_name = "storage")
        {
            if (initiated)
            {
                ESP_LOGW(TAG_SETTINGS, "Settings already initialized");
                return get();
            }
            SpeedSettings &instance = get();
            instance.ns_name = ns_name;
            return instance;
        }

        template <typename TItem>
        inline bool get_item(const char *key, TItem &item)
        {
            esp_err_t err;
            // Handle will automatically close when going out of scope or when it's reset.
            auto handle = nvs::open_nvs_handle(ns_name.begin(), NVS_READONLY, &err);

            ESP_ERROR_CHECK(err);
            err = handle->get_item(key, item);
            if (err == ESP_ERR_NVS_NOT_FOUND)
            {
                return false;
            }
            ESP_ERROR_CHECK(err);
            return true;
        }

        template <typename TItem>
        inline void set_item(const char *key, TItem &item)
        {
            esp_err_t err;
            auto handle = nvs::open_nvs_handle(ns_name.begin(), NVS_READWRITE, &err);
            ESP_ERROR_CHECK(err);
            err = handle->set_item(key, item);
            ESP_ERROR_CHECK(err);
            err = handle->commit();
            ESP_ERROR_CHECK(err);
        }

        template <typename TObject>
        bool get_object(const char *key, TObject *object)
        {
            esp_err_t err;
            auto handle = nvs::open_nvs_handle(ns_name.begin(), NVS_READWRITE, &err);
            ESP_ERROR_CHECK(err);
            err = handle->get_blob(key, object, sizeof(TObject));
            if (err == ESP_ERR_NVS_NOT_FOUND)
            {
                return false;
            }
            ESP_ERROR_CHECK(err);
            return true;
        }

        template <typename TObject>
        void set_object(const char *key, TObject *object)
        {
            esp_err_t err;
            auto handle = nvs::open_nvs_handle(ns_name.begin(), NVS_READWRITE, &err);
            ESP_ERROR_CHECK(err);
            err = handle->set_blob(key, object, sizeof(TObject));
            ESP_ERROR_CHECK(err);
            err = handle->commit();
            ESP_ERROR_CHECK(err);
        }
    };
};

#endif // __SPEED_SETTINGS_H__
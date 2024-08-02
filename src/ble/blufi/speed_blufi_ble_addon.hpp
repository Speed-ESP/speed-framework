#pragma once
#ifndef __SPEED_BLUFI_BLE_ADDON_H__
#define __SPEED_BLUFI_BLE_ADDON_H__

#include "esp_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_blufi.h"
#include "esp_blufi_api.h"

#include "esp_mac.h"
#include "esp_wifi.h"

#include "esp_err.h"
#include "esp_log.h"

#ifdef CONFIG_BT_BLUEDROID_ENABLED
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#endif

#ifdef CONFIG_BT_NIMBLE_ENABLED
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "console/console.h"
#endif

#include "../speed_ble.hpp"
#include "speed_blufi.h"

namespace Speed::BLE::Blufi
{

    class SpeedBlufiCallback
    {
    };

    class SpeedBlufiAddon : public GattAddon
    {
    private:
        class SpeedBluFiAdvertise : public GattAdvertise
        {
            SpeedBluFiAdvertise(std::string_view device_name)
            {
            }

        protected:
            int advertise(struct ble_gap_event *event) override
            {
                /* Handle gap events for BluFi */
                blufi_handle_gap_events(event);
                return ESP_OK;
            }
        };

    private:
        EventGroupHandle_t wifi_event_group;
        bool gl_sta_connected = false;
        bool gl_sta_got_ip = false;
        bool ble_is_connected = false;
        uint8_t gl_sta_bssid[6];
        uint8_t gl_sta_ssid[32];
        int gl_sta_ssid_len;
        wifi_sta_list_t gl_sta_list;
        bool gl_sta_is_connecting = false;
        esp_blufi_extra_info_t gl_sta_conn_info;
        uint8_t blufi_wifi_retry = 0;

        wifi_config_t sta_config;
        wifi_config_t ap_config;

    private:
        void initialise_wifi();
        void wifi_event_handler(esp_event_base_t event_base,
                                int32_t event_id, void *event_data);
        static void wifi_event_handler_wrap(void *arg, esp_event_base_t event_base,
                                            int32_t event_id, void *event_data);
        void ip_event_handler(esp_event_base_t event_base,
                              int32_t event_id, void *event_data);

        static void ip_event_handler_wrap(void *arg, esp_event_base_t event_base,
                                          int32_t event_id, void *event_data);

        int softap_get_current_connection_number();
        bool blufi_wifi_reconnect();

        void blufi_record_wifi_conn_info(int rssi, uint8_t reason);

        void blufi_wifi_connect();

    public:
        explicit SpeedBlufiAddon(std::string name) : GattAddon(name)
        {
        }
        static SpeedBlufiAddon *get_instance()
        {
            static SpeedBlufiAddon instance = SpeedBlufiAddon(BLUFI_DEVICE_NAME);
            return &instance;
        }

        static void event_callback_wrapper(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param)
        {
            get_instance()->event_callback(event, param);
        }

        void event_callback(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param)
        {
            /* actually, should post to blufi_task handle the procedure,
             * now, as a example, we do it more simply */
            switch (event)
            {
            case ESP_BLUFI_EVENT_INIT_FINISH:
                BLUFI_INFO("BLUFI init finish\n");

                blufi_start_advertise();
                break;
            case ESP_BLUFI_EVENT_DEINIT_FINISH:
                BLUFI_INFO("BLUFI deinit finish\n");
                break;
            case ESP_BLUFI_EVENT_BLE_CONNECT:
                BLUFI_INFO("BLUFI ble connect\n");
                ble_is_connected = true;
                blufi_stop_advertise();
                blufi_security_init();
                break;
            case ESP_BLUFI_EVENT_BLE_DISCONNECT:
                BLUFI_INFO("BLUFI ble disconnect\n");
                ble_is_connected = false;
                blufi_security_deinit();
                blufi_start_advertise();
                break;
            case ESP_BLUFI_EVENT_SET_WIFI_OPMODE:
                BLUFI_INFO("BLUFI Set WIFI opmode %d\n", param->wifi_mode.op_mode);
                ESP_ERROR_CHECK(esp_wifi_set_mode(param->wifi_mode.op_mode));
                break;
            case ESP_BLUFI_EVENT_REQ_CONNECT_TO_AP:
                BLUFI_INFO("BLUFI requset wifi connect to AP\n");
                /* there is no wifi callback when the device has already connected to this wifi
                so disconnect wifi before connection.
                */
                esp_wifi_disconnect();
                blufi_wifi_connect();
                break;
            case ESP_BLUFI_EVENT_REQ_DISCONNECT_FROM_AP:
                BLUFI_INFO("BLUFI requset wifi disconnect from AP\n");
                esp_wifi_disconnect();
                break;
            case ESP_BLUFI_EVENT_REPORT_ERROR:
                BLUFI_ERROR("BLUFI report error, error code %d\n", param->report_error.state);
                esp_blufi_send_error_info(param->report_error.state);
                break;
            case ESP_BLUFI_EVENT_GET_WIFI_STATUS:
            {
                wifi_mode_t mode;
                esp_blufi_extra_info_t info;

                esp_wifi_get_mode(&mode);

                if (gl_sta_connected)
                {
                    memset(&info, 0, sizeof(esp_blufi_extra_info_t));
                    memcpy(info.sta_bssid, gl_sta_bssid, 6);
                    info.sta_bssid_set = true;
                    info.sta_ssid = gl_sta_ssid;
                    info.sta_ssid_len = gl_sta_ssid_len;
                    esp_blufi_send_wifi_conn_report(mode, gl_sta_got_ip ? ESP_BLUFI_STA_CONN_SUCCESS : ESP_BLUFI_STA_NO_IP, softap_get_current_connection_number(), &info);
                }
                else if (gl_sta_is_connecting)
                {
                    esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONNECTING, softap_get_current_connection_number(), &gl_sta_conn_info);
                }
                else
                {
                    esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_FAIL, softap_get_current_connection_number(), &gl_sta_conn_info);
                }
                BLUFI_INFO("BLUFI get wifi status from AP\n");

                break;
            }
            case ESP_BLUFI_EVENT_RECV_SLAVE_DISCONNECT_BLE:
                BLUFI_INFO("blufi close a gatt connection");
                blufi_disconnect();
                break;
            case ESP_BLUFI_EVENT_DEAUTHENTICATE_STA:

                break;
            case ESP_BLUFI_EVENT_RECV_STA_BSSID:
                memcpy(sta_config.sta.bssid, param->sta_bssid.bssid, 6);
                sta_config.sta.bssid_set = 1;
                esp_wifi_set_config(WIFI_IF_STA, &sta_config);
                BLUFI_INFO("Recv STA BSSID %s\n", sta_config.sta.ssid);
                break;
            case ESP_BLUFI_EVENT_RECV_STA_SSID:
                strncpy((char *)sta_config.sta.ssid, (char *)param->sta_ssid.ssid, param->sta_ssid.ssid_len);
                sta_config.sta.ssid[param->sta_ssid.ssid_len] = '\0';
                esp_wifi_set_config(WIFI_IF_STA, &sta_config);
                BLUFI_INFO("Recv STA SSID %s\n", sta_config.sta.ssid);
                break;
            case ESP_BLUFI_EVENT_RECV_STA_PASSWD:
                strncpy((char *)sta_config.sta.password, (char *)param->sta_passwd.passwd, param->sta_passwd.passwd_len);
                sta_config.sta.password[param->sta_passwd.passwd_len] = '\0';
                esp_wifi_set_config(WIFI_IF_STA, &sta_config);
                BLUFI_INFO("Recv STA PASSWORD %s\n", sta_config.sta.password);
                break;
            case ESP_BLUFI_EVENT_RECV_SOFTAP_SSID:
                strncpy((char *)ap_config.ap.ssid, (char *)param->softap_ssid.ssid, param->softap_ssid.ssid_len);
                ap_config.ap.ssid[param->softap_ssid.ssid_len] = '\0';
                ap_config.ap.ssid_len = param->softap_ssid.ssid_len;
                esp_wifi_set_config(WIFI_IF_AP, &ap_config);
                BLUFI_INFO("Recv SOFTAP SSID %s, ssid len %d\n", ap_config.ap.ssid, ap_config.ap.ssid_len);
                break;
            case ESP_BLUFI_EVENT_RECV_SOFTAP_PASSWD:
                strncpy((char *)ap_config.ap.password, (char *)param->softap_passwd.passwd, param->softap_passwd.passwd_len);
                ap_config.ap.password[param->softap_passwd.passwd_len] = '\0';
                esp_wifi_set_config(WIFI_IF_AP, &ap_config);
                BLUFI_INFO("Recv SOFTAP PASSWORD %s len = %d\n", ap_config.ap.password, param->softap_passwd.passwd_len);
                break;
            case ESP_BLUFI_EVENT_RECV_SOFTAP_MAX_CONN_NUM:
                if (param->softap_max_conn_num.max_conn_num > 4)
                {
                    return;
                }
                ap_config.ap.max_connection = param->softap_max_conn_num.max_conn_num;
                esp_wifi_set_config(WIFI_IF_AP, &ap_config);
                BLUFI_INFO("Recv SOFTAP MAX CONN NUM %d\n", ap_config.ap.max_connection);
                break;
            case ESP_BLUFI_EVENT_RECV_SOFTAP_AUTH_MODE:
                if (param->softap_auth_mode.auth_mode >= WIFI_AUTH_MAX)
                {
                    return;
                }
                ap_config.ap.authmode = param->softap_auth_mode.auth_mode;
                esp_wifi_set_config(WIFI_IF_AP, &ap_config);
                BLUFI_INFO("Recv SOFTAP AUTH MODE %d\n", ap_config.ap.authmode);
                break;
            case ESP_BLUFI_EVENT_RECV_SOFTAP_CHANNEL:
                if (param->softap_channel.channel > 13)
                {
                    return;
                }
                ap_config.ap.channel = param->softap_channel.channel;
                esp_wifi_set_config(WIFI_IF_AP, &ap_config);
                BLUFI_INFO("Recv SOFTAP CHANNEL %d\n", ap_config.ap.channel);
                break;
            case ESP_BLUFI_EVENT_GET_WIFI_LIST:
            {
                wifi_scan_config_t scanConf = {
                    .ssid = NULL,
                    .bssid = NULL,
                    .channel = 0,
                    .show_hidden = false};
                esp_err_t ret = esp_wifi_scan_start(&scanConf, true);
                if (ret != ESP_OK)
                {
                    esp_blufi_send_error_info(ESP_BLUFI_WIFI_SCAN_FAIL);
                }
                break;
            }
            case ESP_BLUFI_EVENT_RECV_CUSTOM_DATA:
                BLUFI_INFO("Recv Custom Data %" PRIu32 "\n", param->custom_data.data_len);
                ESP_LOG_BUFFER_HEX("Custom Data", param->custom_data.data, param->custom_data.data_len);
                break;
            case ESP_BLUFI_EVENT_RECV_USERNAME:
                /* Not handle currently */
                break;
            case ESP_BLUFI_EVENT_RECV_CA_CERT:
                /* Not handle currently */
                break;
            case ESP_BLUFI_EVENT_RECV_CLIENT_CERT:
                /* Not handle currently */
                break;
            case ESP_BLUFI_EVENT_RECV_SERVER_CERT:
                /* Not handle currently */
                break;
            case ESP_BLUFI_EVENT_RECV_CLIENT_PRIV_KEY:
                /* Not handle currently */
                break;
                ;
            case ESP_BLUFI_EVENT_RECV_SERVER_PRIV_KEY:
                /* Not handle currently */
                break;
            default:
                break;
            }
        }

    public:
        static SpeedBlufiAddon *setup()
        {
            return get_instance();
        }

        std::string_view get_name() override
        {
            return "SPEED Blufi";
        }

        void on_sync() override
        {
            BLUFI_INFO("Initializing blufi profile");
            esp_blufi_profile_init();
        }

        void gatts_register_callback(struct ble_gatt_register_ctxt *ctxt, void *args) override
        {
            blufi_gatt_svr_register_cb(ctxt, args);
        }

        void init(SpeedBLE *driver) override
        {
            GattAddon::init(driver);
            BLE_INFO("Initialize blufi wifi");
            initialise_wifi();

            esp_err_t ret;

            BLUFI_INFO("Binding blufi callbacks");
            static esp_blufi_callbacks_t blufi_callbacks = {
                .event_cb = event_callback_wrapper,
                .negotiate_data_handler = blufi_dh_negotiate_data_handler,
                .encrypt_func = blufi_aes_encrypt,
                .decrypt_func = blufi_aes_decrypt,
                .checksum_func = blufi_crc_checksum,
            };
            ret = esp_blufi_host_and_cb_init(&blufi_callbacks);
            if (ret)
            {
                esp_err_to_name(ret);
                BLUFI_ERROR("%s initialise failed: %s\n", __func__, esp_err_to_name(ret));
                ESP_ERROR_CHECK(ret);
                return;
            }

            BLUFI_INFO("BLUFI VERSION %04x\n", esp_blufi_get_version());
        }

        void start() override
        {
            BLE_INFO("Init esp_blufi");
            blufi_btc_init();
        }
    };
}

#endif // __SPEED_BLUFI_BLE_ADDON_H__
#include "speed_blufi_ble_addon.hpp"

#define WIFI_LIST_NUM 10
const int CONNECTED_BIT = BIT0;
using namespace Speed::BLE::Blufi;
void SpeedBlufiAddon::blufi_record_wifi_conn_info(int rssi, uint8_t reason)
{
    memset(&gl_sta_conn_info, 0, sizeof(esp_blufi_extra_info_t));
    if (gl_sta_is_connecting)
    {
        gl_sta_conn_info.sta_max_conn_retry_set = true;
        gl_sta_conn_info.sta_max_conn_retry = SPEED_FRAMEWORK_BLUFI_WIFI_CON_MAX_RETRY;
    }
    else
    {
        gl_sta_conn_info.sta_conn_rssi_set = true;
        gl_sta_conn_info.sta_conn_rssi = rssi;
        gl_sta_conn_info.sta_conn_end_reason_set = true;
        gl_sta_conn_info.sta_conn_end_reason = reason;
    }
}

void SpeedBlufiAddon::blufi_wifi_connect()
{
    blufi_wifi_retry = 0;
    gl_sta_is_connecting = (esp_wifi_connect() == ESP_OK);
    blufi_record_wifi_conn_info(BLUFI_INVALID_RSSI, BLUFI_INVALID_REASON);
}

bool SpeedBlufiAddon::blufi_wifi_reconnect()
{
    bool ret;
    if (gl_sta_is_connecting && blufi_wifi_retry++ < SPEED_FRAMEWORK_BLUFI_WIFI_CON_MAX_RETRY)
    {
        BLUFI_INFO("BLUFI WiFi starts reconnection\n");
        gl_sta_is_connecting = (esp_wifi_connect() == ESP_OK);
        blufi_record_wifi_conn_info(BLUFI_INVALID_RSSI, BLUFI_INVALID_REASON);
        ret = true;
    }
    else
    {
        ret = false;
    }
    return ret;
}

int SpeedBlufiAddon::softap_get_current_connection_number()
{
    esp_err_t ret;
    ret = esp_wifi_ap_get_sta_list(&gl_sta_list);
    if (ret == ESP_OK)
    {
        return gl_sta_list.num;
    }

    return 0;
}

void SpeedBlufiAddon::ip_event_handler(esp_event_base_t event_base,
                                       int32_t event_id, void *event_data)
{
    wifi_mode_t mode;

    switch (event_id)
    {
    case IP_EVENT_STA_GOT_IP:
    {
        esp_blufi_extra_info_t info;

        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        esp_wifi_get_mode(&mode);

        memset(&info, 0, sizeof(esp_blufi_extra_info_t));
        memcpy(info.sta_bssid, gl_sta_bssid, 6);
        info.sta_bssid_set = true;
        info.sta_ssid = gl_sta_ssid;
        info.sta_ssid_len = gl_sta_ssid_len;
        gl_sta_got_ip = true;
        if (ble_is_connected == true)
        {
            esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_SUCCESS, softap_get_current_connection_number(), &info);
        }
        else
        {
            BLUFI_INFO("BLUFI BLE is not connected yet\n");
        }
        break;
    }
    default:
        break;
    }
    return;
}

void SpeedBlufiAddon::ip_event_handler_wrap(void *arg, esp_event_base_t event_base,
                                            int32_t event_id, void *event_data)
{
    auto *addon = (SpeedBlufiAddon *)arg;
    addon->ip_event_handler(event_base, event_id, event_data);
}

void SpeedBlufiAddon::wifi_event_handler(esp_event_base_t event_base,
                                         int32_t event_id, void *event_data)
{
    wifi_event_sta_connected_t *event;
    wifi_event_sta_disconnected_t *disconnected_event;
    wifi_mode_t mode;
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        blufi_wifi_connect();
        break;
    case WIFI_EVENT_STA_CONNECTED:
        gl_sta_connected = true;
        gl_sta_is_connecting = false;
        event = (wifi_event_sta_connected_t *)event_data;
        memcpy(gl_sta_bssid, event->bssid, 6);
        memcpy(gl_sta_ssid, event->ssid, event->ssid_len);
        gl_sta_ssid_len = event->ssid_len;
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        /* Only handle reconnection during connecting */
        if (gl_sta_connected == false && blufi_wifi_reconnect() == false)
        {
            gl_sta_is_connecting = false;
            disconnected_event = (wifi_event_sta_disconnected_t *)event_data;
            blufi_record_wifi_conn_info(disconnected_event->rssi, disconnected_event->reason);
        }
        /* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
        gl_sta_connected = false;
        gl_sta_got_ip = false;
        memset(gl_sta_ssid, 0, 32);
        memset(gl_sta_bssid, 0, 6);
        gl_sta_ssid_len = 0;
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    case WIFI_EVENT_AP_START:
        esp_wifi_get_mode(&mode);

        /* TODO: get config or information of softap, then set to report extra_info */
        if (ble_is_connected == true)
        {
            if (gl_sta_connected)
            {
                esp_blufi_extra_info_t info;
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
        }
        else
        {
            BLUFI_INFO("BLUFI BLE is not connected yet\n");
        }
        break;
    case WIFI_EVENT_SCAN_DONE:
    {
        uint16_t apCount = 0;
        esp_wifi_scan_get_ap_num(&apCount);
        if (apCount == 0)
        {
            BLUFI_INFO("Nothing AP found");
            break;
        }
        wifi_ap_record_t *ap_list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * apCount);
        if (!ap_list)
        {
            BLUFI_ERROR("malloc error, ap_list is NULL");
            esp_wifi_clear_ap_list();
            break;
        }
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&apCount, ap_list));
        esp_blufi_ap_record_t *blufi_ap_list = (esp_blufi_ap_record_t *)malloc(apCount * sizeof(esp_blufi_ap_record_t));
        if (!blufi_ap_list)
        {
            if (ap_list)
            {
                free(ap_list);
            }
            BLUFI_ERROR("malloc error, blufi_ap_list is NULL");
            break;
        }
        for (int i = 0; i < apCount; ++i)
        {
            blufi_ap_list[i].rssi = ap_list[i].rssi;
            memcpy(blufi_ap_list[i].ssid, ap_list[i].ssid, sizeof(ap_list[i].ssid));
        }

        if (ble_is_connected == true)
        {
            esp_blufi_send_wifi_list(apCount, blufi_ap_list);
        }
        else
        {
            BLUFI_INFO("BLUFI BLE is not connected yet\n");
        }

        esp_wifi_scan_stop();
        free(ap_list);
        free(blufi_ap_list);
        break;
    }
    case WIFI_EVENT_AP_STACONNECTED:
    {

        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        BLUFI_INFO("station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);
        break;
    }
    case WIFI_EVENT_AP_STADISCONNECTED:
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        BLUFI_INFO("station " MACSTR " leave, AID=%d, reason=%d", MAC2STR(event->mac), event->aid, event->reason);
        break;
    }

    default:
        break;
    }
    return;
}

void SpeedBlufiAddon::wifi_event_handler_wrap(void *arg, esp_event_base_t event_base,
                                              int32_t event_id, void *event_data)
{
    auto *addon = (SpeedBlufiAddon *)arg;
    addon->wifi_event_handler(event_base, event_id, event_data);
}

void SpeedBlufiAddon::initialise_wifi()
{
    ESP_ERROR_CHECK(esp_netif_init());
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t const *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);
    esp_netif_t const *ap_netif = esp_netif_create_default_wifi_ap();
    assert(ap_netif);
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &SpeedBlufiAddon::wifi_event_handler_wrap, this));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &SpeedBlufiAddon::ip_event_handler_wrap, this));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    blufi_record_wifi_conn_info(BLUFI_INVALID_RSSI, BLUFI_INVALID_REASON);
    ESP_ERROR_CHECK(esp_wifi_start());
}

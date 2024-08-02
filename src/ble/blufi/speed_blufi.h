#pragma once
#ifndef __SPEED_BLUFI_H__
#define __SPEED_BLUFI_H__

#include <esp_blufi_api.h>
#include <host/ble_gap.h>

static const char * SPEED_BLUFI_TAG = "SPEED_BLUFI";
#define BLUFI_INFO(fmt, ...) ESP_LOGI(SPEED_BLUFI_TAG, fmt, ##__VA_ARGS__)
#define BLUFI_ERROR(fmt, ...) ESP_LOGE(SPEED_BLUFI_TAG, fmt, ##__VA_ARGS__)

#define SPEED_FRAMEWORK_BLUFI_WIFI_CON_MAX_RETRY CONFIG_SPEED_FRAMEWORK_BLUFI_WIFI_CON_MAX_RETRY
#define BLUFI_INVALID_REASON                255
#define BLUFI_INVALID_RSSI                  -128

#ifdef __cplusplus
extern "C"
{
#endif
    void blufi_dh_negotiate_data_handler(uint8_t *data, int len, uint8_t **output_data, int *output_len, bool *need_free);
    int blufi_aes_encrypt(uint8_t iv8, uint8_t *crypt_data, int crypt_len);
    int blufi_aes_decrypt(uint8_t iv8, uint8_t *crypt_data, int crypt_len);
    uint16_t blufi_crc_checksum(uint8_t iv8, uint8_t *data, int len);

    int blufi_security_init(void);
    void blufi_security_deinit(void);
    int esp_blufi_gap_register_callback(void);
    esp_err_t esp_blufi_host_init(void);
    esp_err_t esp_blufi_host_and_cb_init(esp_blufi_callbacks_t *callbacks);
    esp_err_t esp_blufi_host_deinit(void);
    esp_err_t esp_blufi_controller_init(void);
    esp_err_t esp_blufi_controller_deinit(void);

    void blufi_disconnect();

    void blufi_start_advertise();

    void blufi_stop_advertise();

    void blufi_gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg);

    void blufi_handle_gap_events(struct ble_gap_event *event);

    void blufi_btc_init();

#ifdef __cplusplus
}
#endif

#endif // __SPEED_BLUFI_H__
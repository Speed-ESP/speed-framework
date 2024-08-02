/*
 * SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <stdio.h>
#include "esp_err.h"
#include "esp_blufi_api.h"
#include "esp_log.h"
#include "esp_blufi.h"
#include "speed_blufi.h"
#if CONFIG_BT_CONTROLLER_ENABLED || !CONFIG_BT_NIMBLE_ENABLED
#include "esp_bt.h"
#endif
#ifdef CONFIG_BT_BLUEDROID_ENABLED
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

#ifdef CONFIG_BT_BLUEDROID_ENABLED
esp_err_t esp_blufi_host_init(void)
{
    int ret;
    ret = esp_bluedroid_init();
    if (ret)
    {
        BLUFI_ERROR("%s init bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return ESP_FAIL;
    }

    ret = esp_bluedroid_enable();
    if (ret)
    {
        BLUFI_ERROR("%s init bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return ESP_FAIL;
    }
    BLUFI_INFO("BD ADDR: " ESP_BD_ADDR_STR "\n", ESP_BD_ADDR_HEX(esp_bt_dev_get_address()));

    return ESP_OK;
}

esp_err_t esp_blufi_host_deinit(void)
{
    int ret;
    ret = esp_blufi_profile_deinit();
    if (ret != ESP_OK)
    {
        return ret;
    }

    ret = esp_bluedroid_disable();
    if (ret)
    {
        BLUFI_ERROR("%s deinit bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return ESP_FAIL;
    }

    ret = esp_bluedroid_deinit();
    if (ret)
    {
        BLUFI_ERROR("%s deinit bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t esp_blufi_gap_register_callback(void)
{
    int rc;
    rc = esp_ble_gap_register_callback(esp_blufi_gap_event_handler);
    if (rc)
    {
        return rc;
    }
    return esp_blufi_profile_init();
}

esp_err_t esp_blufi_host_and_cb_init(esp_blufi_callbacks_t *example_callbacks)
{
    esp_err_t ret = ESP_OK;

    ret = esp_blufi_host_init();
    if (ret)
    {
        BLUFI_ERROR("%s initialise host failed: %s\n", __func__, esp_err_to_name(ret));
        return ret;
    }

    ret = esp_blufi_register_callbacks(example_callbacks);
    if (ret)
    {
        BLUFI_ERROR("%s blufi register failed, error code = %x\n", __func__, ret);
        return ret;
    }

    ret = esp_blufi_gap_register_callback();
    if (ret)
    {
        BLUFI_ERROR("%s gap register failed, error code = %x\n", __func__, ret);
        return ret;
    }

    return ESP_OK;
}

#endif /* CONFIG_BT_BLUEDROID_ENABLED */

#if CONFIG_BT_CONTROLLER_ENABLED || !CONFIG_BT_NIMBLE_ENABLED
esp_err_t esp_blufi_controller_init()
{
    esp_err_t ret = ESP_OK;
// #if CONFIG_IDF_TARGET_ESP32
//     ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
// #endif

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret)
    {
        BLUFI_ERROR("%s initialize bt controller failed: %s\n", __func__, esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret)
    {
        BLUFI_ERROR("%s enable bt controller failed: %s\n", __func__, esp_err_to_name(ret));
        return ret;
    }
    return ret;
}
#endif

#if CONFIG_BT_CONTROLLER_ENABLED || !CONFIG_BT_NIMBLE_ENABLED
esp_err_t esp_blufi_controller_deinit()
{
    esp_err_t ret = ESP_OK;
    ret = esp_bt_controller_disable();
    if (ret)
    {
        BLUFI_ERROR("%s disable bt controller failed: %s\n", __func__, esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bt_controller_deinit();
    if (ret)
    {
        BLUFI_ERROR("%s deinit bt controller failed: %s\n", __func__, esp_err_to_name(ret));
        return ret;
    }

    return ret;
}
#endif

#ifdef CONFIG_BT_NIMBLE_ENABLED

esp_err_t esp_blufi_host_init(void)
{

    BLUFI_INFO("Initializing BluFi gatt services");
    int rc;
    rc = esp_blufi_gatt_svr_init();
    assert(rc == 0);

    BLUFI_INFO("Set blufi device name to %s", BLUFI_DEVICE_NAME);
    /* Set the default device name. */
    rc = ble_svc_gap_device_name_set(BLUFI_DEVICE_NAME);
    assert(rc == 0);

    return ESP_OK;
}

esp_err_t esp_blufi_host_deinit(void)
{
    esp_err_t ret = ESP_OK;

    ret = esp_blufi_profile_deinit();
    if (ret != ESP_OK)
    {
        return ret;
    }

    esp_blufi_btc_deinit();

    ret = nimble_port_stop();
    if (ret == 0)
    {
        esp_nimble_deinit();
    }

    return ret;
}

esp_err_t esp_blufi_gap_register_callback(void)
{
    return ESP_OK;
}

esp_err_t esp_blufi_host_and_cb_init(esp_blufi_callbacks_t *callbacks)
{
    esp_err_t ret = ESP_OK;

    ret = esp_blufi_register_callbacks(callbacks);
    if (ret)
    {
        BLUFI_ERROR("%s blufi register failed, error code = %x\n", __func__, ret);
        return ret;
    }

    ret = esp_blufi_gap_register_callback();
    if (ret)
    {
        BLUFI_ERROR("%s gap register failed, error code = %x\n", __func__, ret);
        return ret;
    }

    ret = esp_blufi_host_init();
    if (ret)
    {
        BLUFI_ERROR("%s initialise host failed: %s\n", __func__, esp_err_to_name(ret));
        return ret;
    }

    return ret;
}

#endif /* CONFIG_BT_NIMBLE_ENABLED */

void blufi_disconnect()
{
    esp_blufi_disconnect();
}

void blufi_start_advertise()
{
    esp_blufi_adv_start();
}

void blufi_stop_advertise()
{
    esp_blufi_adv_stop();
}

void blufi_gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg)
{
    esp_blufi_gatt_svr_register_cb(ctxt, arg);
}

void blufi_handle_gap_events(struct ble_gap_event *event)
{
    esp_blufi_handle_gap_events(event, NULL);
}

void blufi_btc_init()
{
    esp_blufi_btc_init();
}
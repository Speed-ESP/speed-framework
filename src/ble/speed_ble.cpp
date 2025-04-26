#include "speed_ble.hpp"
#include "host/ble_hs.h"

#include <settings/speed_settings.hpp>


void ble_print_conn_desc(struct ble_gap_conn_desc *desc)
{
    MODLOG_DFLT(INFO, "handle=%d our_ota_addr_type=%d our_ota_addr=",
                desc->conn_handle, desc->our_ota_addr.type);
    print_addr(desc->our_ota_addr.val);
    MODLOG_DFLT(INFO, " our_id_addr_type=%d our_id_addr=",
                desc->our_id_addr.type);
    print_addr(desc->our_id_addr.val);
    MODLOG_DFLT(INFO, " peer_ota_addr_type=%d peer_ota_addr=",
                desc->peer_ota_addr.type);
    print_addr(desc->peer_ota_addr.val);
    MODLOG_DFLT(INFO, " peer_id_addr_type=%d peer_id_addr=",
                desc->peer_id_addr.type);
    print_addr(desc->peer_id_addr.val);
    MODLOG_DFLT(INFO, " conn_itvl=%d conn_latency=%d supervision_timeout=%d "
                      "encrypted=%d authenticated=%d bonded=%d\n",
                desc->conn_itvl, desc->conn_latency,
                desc->supervision_timeout,
                desc->sec_state.encrypted,
                desc->sec_state.authenticated,
                desc->sec_state.bonded);
}

void Speed::BLE::SpeedBLE::configure(std::string_view device_name)
{
    Speed::Settings::SpeedSettings::setup();
    esp_err_t ret = nimble_port_init();
    if (ret != ESP_OK)
    {
        BLE_ERROR("Failed to init nimble %d ", ret);
        ESP_ERROR_CHECK(ret);
        return;
    }

    BLE_INFO("Binding callbacks");
    ble_hs_cfg.gatts_register_arg = this;
    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.reset_cb = ble_on_reset;
    ble_hs_cfg.sync_cb = static_ble_on_sync;
    ble_hs_cfg.gatts_register_cb = gatt_register_cb;
    ble_hs_cfg.store_status_cb = store_status_callback;
    ble_hs_cfg.sm_io_cap = BLE_SM_IO_CAP_KEYBOARD_DISP;

    ble_hs_cfg.sm_bonding = 1;
    ble_hs_cfg.sm_mitm = 1;
    ble_hs_cfg.sm_sc = 1;
    ble_hs_cfg.sm_sc = 0;
    ble_hs_cfg.sm_our_key_dist = BLE_SM_PAIR_KEY_DIST_ENC;
    ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ENC;
    register_services();

    /* Set the default device name. */
    int rc = ble_svc_gap_device_name_set(device_name.begin());
    assert(rc == 0);

    BLE_INFO("Initializing addons, total: %d", _addons.size());
    for (auto addon : _addons)
    {
        BLE_INFO("Initializing addon: %s", addon->get_name().begin());
        addon->init(this);
    }

    BLE_INFO("Initialize ble store");
    ble_store_config_init();
}
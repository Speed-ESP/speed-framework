#pragma once
#ifndef H_SPEED_BLE_
#define H_SPEED_BLE_

#if !defined(CONFIG_SPEED_FRAMEWORK_USE_BLE)
    #error "Please enable BLE Speed Framework by running menuconfig"
#endif
#include <vector>
#include <functional>

#include <memory>
#include <stdio.h>

#include "nvs_flash.h"

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
#endif

/* BLE */
#include "host/ble_uuid.h"
#include "services/ans/ble_svc_ans.h"
#include "host/util/util.h"
#include "host/ble_store.h"

#include "core/singleton_service.hpp"
#include "core/enum_name.h"

constexpr char *TAG_BLE = "SPEED-BLE";

#define BLE_INFO(fmt, ...) ESP_LOGI(TAG_BLE, fmt, ##__VA_ARGS__)
#define BLE_ERROR(fmt, ...) ESP_LOGE(TAG_BLE, fmt, ##__VA_ARGS__)
extern "C" void ble_store_config_init(void);
#define BLE_ERROR_CHECK(x, message)    \
    if (x != 0)                        \
    {                                  \
        ESP_LOGE(TAG_BLE, message, x); \
        return x;                      \
    }

static void print_addr(const uint8_t *addr)
{
    const uint8_t *u8p;

    u8p = addr;
    MODLOG_DFLT(INFO, "%02x:%02x:%02x:%02x:%02x:%02x",
                u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);
}

template <class TService, class Method, Method m, class... Params>
static auto bounce(void *priv, Params... params) -> decltype(((*reinterpret_cast<TService *>(priv)).*m)(params...))
{
    return ((*reinterpret_cast<TService *>(priv)).*m)(params...);
}
#define BOUNCE(c, m) bounce<c, decltype(&c::m), &c::m>

 void ble_print_conn_desc(struct ble_gap_conn_desc *desc);

namespace Speed::BLE
{

    using namespace Speed::Core;
    class SpeedBLE;

    class GattObject
    {
    protected:
        explicit GattObject(ble_uuid_t *uuid) : uuid(uuid)
        {
        }
        const char *get_string_uuid() const
        {
            char buf[BLE_UUID_STR_LEN];
            return ble_uuid_to_str(uuid, buf);
        }

    public:
        ble_uuid_t *uuid;
    };
    template <typename TChild, typename TDefinition>
    class ParentGattObject : public GattObject
    {
    private:
        std::vector<TChild *> childs;

    public:
        using GattObject::GattObject;

    protected:
        std::vector<TChild *> *get_childs()
        {
            return &childs;
        }
        inline TDefinition *define_childs()
        {
            BLE_INFO("Defining %d childs", childs.size());
            if (childs.size() == 0)
            {
                return nullptr;
            }
            auto *definitions = new std::vector<TDefinition>();
            for (auto child : childs)
            {
                auto definition = new TDefinition();
                child->define(definition);
                definitions->push_back(*definition);
            }
            static TDefinition empty = {0};
            definitions->push_back(empty);
            return &(*definitions)[0];
        }
    };

    class GattDescriptor : public GattObject
    {
    private:
        static int access_callback(uint16_t conn_handle,
                                   uint16_t attr_handle,
                                   struct ble_gatt_access_ctxt *ctxt,
                                   void *arg)
        {
            auto *descriptor = (GattDescriptor *)arg;
            return descriptor->access(conn_handle, attr_handle, ctxt);
        }

    protected:
        inline uint8_t get_flags()
        {
            return 0;
        }
        virtual int access(uint16_t conn_handle,
                           uint16_t attr_handle,
                           struct ble_gatt_access_ctxt *ctxt)
        {
            return 0;
        }

        virtual void on_setup(ble_gatt_dsc_def *def)
        {
        }

    public:
        ble_gatt_dsc_def *define(ble_gatt_dsc_def *def)
        {
            BLE_INFO("Defining descriptor %s, flags: %d", get_string_uuid(), get_flags());
            def->uuid = uuid;
            def->att_flags = get_flags();
            def->access_cb = access_callback;
            def->arg = this;
            on_setup(def);
            return def;
        }

        inline virtual void on_register(uint16_t def_handle)
        {
        }
    };

    class GattCharacteristic : public ParentGattObject<GattDescriptor, ble_gatt_dsc_def>
    {

    public:
        using ParentGattObject::ParentGattObject;
        virtual ~GattCharacteristic() = default;
        ble_gatt_chr_def *define(ble_gatt_chr_def *def)
        {
            BLE_INFO("Defining characteristic %s, flags: %d", get_string_uuid(), get_flags());
            def->uuid = uuid;
            def->access_cb = access_callback;
            def->arg = this;
            def->descriptors = define_childs();
            def->flags = get_flags();
            on_setup(def);
            return def;
        }

        inline virtual void on_register(uint16_t def_handle, uint16_t val_handle)
        {
        }

    protected:
        inline virtual void on_setup(ble_gatt_chr_def *definition)
        {
        }
        inline virtual ble_gatt_chr_flags get_flags()
        {
            return BLE_GATT_CHR_F_READ;
        }
        inline virtual int read(ble_gatt_access_ctxt *ctxt)
        {
            BLE_INFO("Invoking not overridden write callback on characteristic: %s", get_string_uuid());
            return BLE_ATT_ERR_INVALID_HANDLE;
        }

        inline virtual int write(ble_gatt_access_ctxt *ctxt)
        {
            BLE_INFO("Invoking not overridden write callback on characteristic: %s", get_string_uuid());
            return BLE_ATT_ERR_INVALID_HANDLE;
        }

        int raw_write(struct os_mbuf *om,
                      uint16_t min_len,
                      uint16_t max_len,
                      void *dst,
                      uint16_t *len)
        {
            uint16_t om_len;
            int rc;

            om_len = OS_MBUF_PKTLEN(om);
            if (om_len < min_len || om_len > max_len)
            {
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }

            rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
            if (rc != 0)
            {
                return BLE_ATT_ERR_UNLIKELY;
            }

            return 0;
        }

    private:
        static int access_callback(uint16_t conn_handle,
                                   uint16_t attr_handle,
                                   struct ble_gatt_access_ctxt *ctxt,
                                   void *arg)
        {
            ESP_LOGI(TAG_BLE, "Characteristic access callback, operation %d", ctxt->op);
            auto *characteristic = (GattCharacteristic *)arg;
            switch (ctxt->op)
            {
            case BLE_GATT_ACCESS_OP_READ_CHR:
                BLE_INFO("Reading characteristic %s", characteristic->get_string_uuid());
                return characteristic->read(ctxt);
                break;
            case BLE_GATT_ACCESS_OP_WRITE_CHR:
                BLE_INFO("Writing characteristic %s", characteristic->get_string_uuid());
                return characteristic->write(ctxt);
                break;
            case BLE_GATT_ACCESS_OP_READ_DSC:
                BLE_INFO("READ_DSC not implemented yet");
                break;
            case BLE_GATT_ACCESS_OP_WRITE_DSC:
                BLE_INFO("WRITE_DSC not implemented yet");
                break;
            default:
                BLE_INFO("Unkown event");
                assert(0);
                return BLE_ATT_ERR_UNLIKELY;
            }
            return 0;
        }
    };

    DECLARE_ENUM(GattServiceType, uint8_t, SECONDARY, PRIMARY, NONE);

    class GattService : public ParentGattObject<GattCharacteristic, ble_gatt_chr_def>
    {
    protected:
        GattServiceType type;

    protected:
        inline virtual void on_setup(ble_gatt_svc_def *definition)
        {
            BLE_INFO("Executing default on_setup");
        }

    public:
        GattService(ble_uuid_t *uuid, const GattServiceType &type) : ParentGattObject(uuid), type(type)
        {
        }
        virtual ~GattService() = default;

        inline void add_characteristic(GattCharacteristic *characteristic)
        {
            get_childs()->push_back(characteristic);
        }

        inline ble_gatt_svc_def *define(struct ble_gatt_svc_def *def)
        {
            MODLOG_DFLT(INFO, "Defining service %s as type: %s", get_string_uuid(), get_type_name());
            def->type = (uint8_t)type;
            def->uuid = uuid;
            def->characteristics = define_childs();
            on_setup(def);
            return def;
        }

        const char *get_type_name()
        {
            return type.get_name().c_str();
        }
    };

    DECLARE_FLAG_ENUM(GattAdvertiseFlags, uint8_t,
                 LE_LIMITED_DISCOVERY_MODE, BLE_HS_ADV_F_DISC_LTD,
                 LE_GENERAL_DISCOVERY_MODE, BLE_HS_ADV_F_DISC_GEN,
                 BRE_NOT_SUPPORTED, BLE_HS_ADV_F_BREDR_UNSUP);

    DECLARE_ENUM(GattAdvertiseStartMode, uint8_t, ON_START, MANUALLY, ON_SYNC);

    DECLARE_VALUE_ENUM(OwnAddressType, uint8_t, PUBLIC, 0x00, RANDOM, 0x01, RPA_PUBLIC_DEFAULT, 0x03, RPA_RANDOM_DEFAULT, 0x04);

    class GattAdvertise
    {
    protected:
        OwnAddressType own_addr_type;
        bool is_connected = false;
        uint8_t connection_handle = 0;

        GattAdvertise(OwnAddressType own_address_type = OwnAddressType::PUBLIC) : own_addr_type(own_address_type)
        {
        }

        virtual GattAdvertiseFlags get_flags()
        {
            return GattAdvertiseFlags::LE_GENERAL_DISCOVERY_MODE;
        }

        virtual const char *get_name()
        {
            return ble_svc_gap_device_name();
        }

        virtual int get_duration()
        {
            return BLE_HS_FOREVER;
        }

        virtual void configure_fields(ble_hs_adv_fields &fields)
        {
            BLE_INFO("No configure_fields configured");
        }

        virtual void configure_parameters(ble_gap_adv_params &parameters)
        {
            BLE_INFO("No configure_parameters configured");
        }

        virtual const ble_addr_t *get_direct_address()
        {
            return NULL;
        }

        virtual int advertise(struct ble_gap_event *event)
        {
            struct ble_gap_conn_desc desc;
            int rc;
            switch (event->type)
            {
            case BLE_GAP_EVENT_CONNECT:
                /* A new connection was established or a connection attempt failed. */
                BLE_INFO("connection %s; status=%d ",
                         event->connect.status == 0 ? "established" : "failed",
                         event->connect.status);
                if (event->connect.status == 0)
                {
                    rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
                    assert(rc == 0);
                    ble_print_conn_desc(&desc);
                    is_connected = true;
                    connection_handle = event->connect.conn_handle;
                }
                BLE_INFO("\n");
                if (event->connect.status != 0)
                {
                    MODLOG_DFLT(WARN, "Connection failed, start to advertise again");
                    /* Connection failed; resume advertising. */
                    start();
                }
                return 0;

            case BLE_GAP_EVENT_DISCONNECT:
                BLE_INFO("disconnect; reason=%d ", event->disconnect.reason);
                ble_print_conn_desc(&event->disconnect.conn);
                BLE_INFO("\n");
                is_connected = false;
                /* Connection terminated; resume advertising. */
                start();
                return 0;

            case BLE_GAP_EVENT_CONN_UPDATE:
                /* The central has updated the connection parameters. */
                BLE_INFO("connection updated; status=%d ",
                         event->conn_update.status);
                rc = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
                assert(rc == 0);
                ble_print_conn_desc(&desc);
                BLE_INFO("\n");
                return 0;

            case BLE_GAP_EVENT_ADV_COMPLETE:
                BLE_INFO("advertise complete; reason=%d",
                         event->adv_complete.reason);
                start();
                return 0;

            case BLE_GAP_EVENT_ENC_CHANGE:
                /* Encryption has been enabled or disabled for this connection. */
                BLE_INFO("encryption change event; status=%d ",
                         event->enc_change.status);
                rc = ble_gap_conn_find(event->enc_change.conn_handle, &desc);
                assert(rc == 0);
                ble_print_conn_desc(&desc);
                return 0;
            case BLE_GAP_EVENT_SUBSCRIBE:
                BLE_INFO("subscribe event; conn_handle=%d attr_handle=%d "
                         "reason=%d prevn=%d curn=%d previ=%d curi=%d",
                         event->subscribe.conn_handle,
                         event->subscribe.attr_handle,
                         event->subscribe.reason,
                         event->subscribe.prev_notify,
                         event->subscribe.cur_notify,
                         event->subscribe.prev_indicate,
                         event->subscribe.cur_indicate);
                return 0;

            case BLE_GAP_EVENT_MTU:
                BLE_INFO("mtu update event; conn_handle=%d cid=%d mtu=%d\n",
                         event->mtu.conn_handle,
                         event->mtu.channel_id,
                         event->mtu.value);
                return 0;
            case BLE_GAP_EVENT_REPEAT_PAIRING:
                /* We already have a bond with the peer, but it is attempting to
                 * establish a new secure link.  This app sacrifices security for
                 * convenience: just throw away the old bond and accept the new link.
                 */

                /* Delete the old bond. */
                rc = ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
                assert(rc == 0);
                ble_store_util_delete_peer(&desc.peer_id_addr);

                /* Return BLE_GAP_REPEAT_PAIRING_RETRY to indicate that the host should
                 * continue with the pairing operation.
                 */
                return BLE_GAP_REPEAT_PAIRING_RETRY;

            default:
                return 0;
            }
            return 0;
        }

    private:
        static int advertise_callback(struct ble_gap_event *event, void *arg)
        {
            auto *advertise = (GattAdvertise *)arg;
            if (!advertise)
            {
                MODLOG_DFLT(INFO, "Can't convert arg to gatt_advertise, event: %d", event->type);
                return 0;
            }
            MODLOG_DFLT(INFO, "Advertise event received, event: %d", event->type);
            return advertise->advertise(event);
        }

    public:
        virtual GattAdvertiseStartMode get_start_mode()
        {
            return GattAdvertiseStartMode::ON_SYNC;
        }

        void start()
        {
            start(own_addr_type);
        }

        void start(OwnAddressType addr_type)
        {
            MODLOG_DFLT(INFO, "Starting to advertise, addr_type: %s", addr_type.get_c_name());
            this->own_addr_type = addr_type;

            struct ble_gap_adv_params adv_params;
            struct ble_hs_adv_fields fields;
            const char *name;
            int rc;

            memset(&fields, 0, sizeof fields);

            fields.flags = get_flags();

            fields.tx_pwr_lvl_is_present = 1;
            fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

            name = get_name();
            fields.name = (uint8_t *)name;
            fields.name_len = strlen(name);
            fields.name_is_complete = 1;

            configure_fields(fields);
            rc = ble_gap_adv_set_fields(&fields);

            if (rc != 0)
            {
                MODLOG_DFLT(ERROR, "error setting advertisement data; rc=%d\n", rc);
                return;
            }

            /* Begin advertising. */
            memset(&adv_params, 0, sizeof adv_params);
            adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
            adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

            configure_parameters(adv_params);

            rc = ble_gap_adv_start(addr_type,
                                   get_direct_address(),
                                   get_duration(),
                                   &adv_params,
                                   advertise_callback,
                                   this);
            if (rc != 0)
            {
                MODLOG_DFLT(ERROR, "error enabling advertisement; rc=%d\n", rc);
                return;
            }
        }
    };

    class GattAddon
    {
    private:
        SpeedBLE *_driver;
        std::string_view name;

    protected:
        SpeedBLE *get_driver()
        {
            return _driver;
        }

        explicit GattAddon(std::string_view name) : name(name)
        {
        }

        virtual ~GattAddon() = default;

    public:
        virtual void on_sync()
        {
            BLE_INFO("Callign Addon base on_sync method");
        }

        virtual void on_reset(int reason)
        {
            BLE_INFO("Callign Addon base on_reset method");
        }

        virtual void gatts_register_callback(struct ble_gatt_register_ctxt *ctxt, void *args)
        {
            BLE_INFO("Callign Addon base gatts_register_callback method");
        }

        virtual void store_status_callback(struct ble_store_status_event *event, void *args)
        {
            BLE_INFO("Callign Addon base store_status_callback method");
        }

        virtual std::string_view get_name()
        {
            return name;
        }

        virtual void register_services()
        {
            BLE_INFO("Callign Addon base register_services method");
        }

        virtual void init(SpeedBLE *driver)
        {
            _driver = driver;
        }

        virtual void start()
        {
            BLE_INFO("Callign Addon base start method");
        }
    };

    class SpeedBLE : SingletonService<SpeedBLE>
    {
    private:
        struct ble_gatt_register_ctxt register_context;
        std::vector<GattService *> services;
        std::vector<GattAdvertise *> advertises;
        std::vector<GattAddon *> _addons;

        inline ble_gatt_svc_def *define_services()
        {
            static std::vector<ble_gatt_svc_def> *definitions = new std::vector<ble_gatt_svc_def>();

            for (auto service : services)
            {
                auto definition = new ble_gatt_svc_def();
                service->define(definition);
                definitions->push_back(*definition);
            }
            static ble_gatt_svc_def empty = {0};
            definitions->push_back(empty);
            return &(*definitions)[0];
        }

        inline int register_services()
        {
            int rc = 0;

            BLE_INFO("Registering default services");
            ble_svc_gap_init();
            ble_svc_gatt_init();
            BLE_INFO("Registering custom %d services", services.size());

            auto service_definitions = define_services();

            BLE_INFO("Configuring services count");
            rc = ble_gatts_count_cfg(service_definitions);
            BLE_ERROR_CHECK(rc, "Can't configure gatt service count, error: %d");

            BLE_INFO("Configuring custom services");
            rc = ble_gatts_add_svcs(service_definitions);
            BLE_ERROR_CHECK(rc, "Can't add BLE services, error: %d");

            for (auto &addon : _addons)
            {
                addon->register_services();
            }
            return ESP_OK;
        }

        void configure(std::string_view device_name);

    public:
        static SpeedBLE *create()
        {
            return &get();
        }

        inline void add_service(GattService *service)
        {
            services.push_back(service);
        }
        inline void add_advertise(GattAdvertise *advertise)
        {
            BLE_INFO("Adding advertising");
            advertises.push_back(advertise);
        }

        inline void add_addon(GattAddon *addon)
        {
            BLE_INFO("Plugin addon: %s", addon->get_name().cbegin());
            _addons.push_back(addon);
        }

        void start(std::string_view device_name)
        {
            BLE_INFO("Starting BLE");
            configure(device_name);
            for (auto &addon : _addons)
            {
                BLE_INFO("Starting addon %s", addon->get_name().cbegin());
                addon->start();
            }

            auto err = esp_nimble_enable((void *)ble_host_task);
            if (err)
            {
                BLE_ERROR("%s failed: %s\n", __func__, esp_err_to_name(err));
                ESP_ERROR_CHECK(err);
            }
        }

    private:
        void ble_on_sync()
        {
            int rc;

            rc = ble_hs_util_ensure_addr(0);
            assert(rc == 0);
            uint8_t own_addr_type;

            /* Figure out address to use while advertising (no privacy for now) */
            rc = ble_hs_id_infer_auto(0, &own_addr_type);
            if (rc != 0)
            {
                MODLOG_DFLT(ERROR, "error determining address type; rc=%d\n", rc);
                return;
            }
            MODLOG_DFLT(INFO, "Advertise address figure out: %d", own_addr_type);
            /* Printing ADDR */
            uint8_t addr_val[6] = {0};
            rc = ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);
            MODLOG_DFLT(INFO, "Device Address: ");
            print_addr(addr_val);
            MODLOG_DFLT(INFO, "\n");
            MODLOG_DFLT(INFO, "Device Name: ");
            MODLOG_DFLT(INFO, "%s", ble_svc_gap_device_name());
            MODLOG_DFLT(INFO, "\n");

            start_advertise(own_addr_type, GattAdvertiseStartMode::ON_SYNC);

            // sync addons
            for (auto &addon : _addons)
            {
                addon->on_sync();
            }
        }

        static int store_status_callback(struct ble_store_status_event *event, void *arg)
        {
            BLE_INFO("Handling store status");

            for (auto &addon : get()._addons)
            {
                addon->store_status_callback(event, arg);
            }

            BLE_INFO("Handling ble_store_util_status_rr");
            return ble_store_util_status_rr(event, arg);
        }

        static void ble_host_task(void *param)
        {
            BLE_INFO("BLE Host Task Started");
            /* This function will return only when nimble_port_stop() is executed */
            nimble_port_run();

            nimble_port_freertos_deinit();
        }

        void start_advertise(uint8_t addr_type, GattAdvertiseStartMode current_mode)
        {
            ESP_LOGD(TAG_BLE, "Starting to advertise items: %d", advertises.size());
            for (auto advertise : advertises)
            {
                if (advertise->get_start_mode() != current_mode)
                {
                    continue;
                }

                advertise->start(addr_type);
            }
        }

        static void static_ble_on_sync()
        {
            BLE_INFO("Executing BLE ON SYNC");
            get().ble_on_sync();
        }

        static void ble_on_reset(int reason)
        {
            BLE_ERROR("Resetting state; reason=%s\n", esp_err_to_name(reason));
            for (auto &addon : get()._addons)
            {
                addon->on_reset(reason);
            }
        }

        static void gatt_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg)
        {
            char buf[BLE_UUID_STR_LEN];
            switch (ctxt->op)
            {
            case BLE_GATT_REGISTER_OP_SVC:
            {
                BLE_INFO("Registered service %s with handle=%d\n",
                         ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                         ctxt->svc.handle);
            }
            break;

            case BLE_GATT_REGISTER_OP_CHR:
            {
                BLE_INFO("Registering characteristic %s with def_handle=%d val_handle=%d\n",
                         ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                         ctxt->chr.def_handle,
                         ctxt->chr.val_handle);
            }

            break;

            case BLE_GATT_REGISTER_OP_DSC:
            {

                BLE_INFO("Registering descriptor %s with handle=%d\n",
                         ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                         ctxt->dsc.handle);
                break;
            }
            }

            for (auto &addon : get()._addons)
            {
                BLE_INFO("Registering gatt addon: %s", addon->get_name().cbegin());
                addon->gatts_register_callback(ctxt, arg);
            }
        }
    };
}
#endif

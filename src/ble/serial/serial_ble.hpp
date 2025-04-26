#pragma once

#ifndef __SERIAL_BLE_H__
#define __SERIAL_BLE_H__


/* BLE */
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "console/console.h"
#include "services/gap/ble_svc_gap.h"
#include <ble/speed_ble.hpp>
#include <host/ble_uuid.h>

#define GATT_SVR_SVC_ALERT_UUID               0x1811
namespace Speed::BLE::Serial
{

    class ProviderBase
    {
    public:
        virtual int available() = 0;
        virtual int read() = 0;
        virtual void write(uint8_t data) = 0;
    };

    template <class TSerialStream>
    class Provider : public ProviderBase
    {
    private:
        TSerialStream &m_stream;

    public:
        Provider(TSerialStream &stream) : m_stream(stream)
        {
        }

        int available() override
        {
            return m_stream.available();
        }

        int read() override
        {
            return m_stream.read();
        }

        void write(uint8_t data) override
        {
            m_stream.write(data);
        }
    };

    /* UUID string: 0DA3D922-38FF-41EF-9467-C2A778821BAA */
    static ble_uuid128_t serialCharacteristicUUID = BLE_UUID128_INIT(0xAA, 0x1B, 0x82, 0x78, 0xA7, 0xC2, 0x67, 0x94, 0xEF, 0x41, 0xFF, 0x38, 0x22, 0xD9, 0xA3, 0x0D);

    class SerialCharacteristic : public GattCharacteristic
    {
    private:
        ProviderBase *m_provider;

    public:
        SerialCharacteristic(ProviderBase *provider) : GattCharacteristic(&serialCharacteristicUUID.u), m_provider(provider)
        {
        }

    public:
        ble_gatt_chr_flags get_flags() override
        {
            return BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE_NO_RSP;
        }

        inline int read(ble_gatt_access_ctxt *ctxt) override
        {
            int available = m_provider->available();
            if (available > 0)
            {
                std::vector<uint8_t> data(available);
                for (int i = 0; i < available; i++)
                {
                    data.push_back(m_provider->read());
                }
                ESP_ERROR_CHECK(os_mbuf_append(ctxt->om, data.data(), data.size()));
                return 1;
            }
            return 0;
        }

        inline int write(ble_gatt_access_ctxt *ctxt) override
        {
            std::vector<uint8_t> data(ctxt->om->om_len);
            ESP_ERROR_CHECK(raw_write(ctxt->om, ctxt->om->om_len, ctxt->om->om_len, data.data(), NULL));
            for (size_t i = 0; i < data.size(); i++)
            {
                m_provider->write(data[i]);
            }
            return 0;
        }
    };

    /* UUID string: 36d370a1-8c9c-4edf-a622-383866fc4924 */
    static ble_uuid128_t serialServiceUUID = BLE_UUID128_INIT(0x24, 0x49, 0xfc, 0x66, 0x38, 0x38, 0x22, 0xa6, 0xdf, 0x4e, 0x9c, 0x8c, 0xa1, 0x70, 0xd3, 0x36);

    class SerialService : public GattService
    {
    public:
        SerialService(ProviderBase *provider) : GattService(&serialServiceUUID.u, GattServiceType::PRIMARY)
        {
            static SerialCharacteristic serialCharacteristic(provider);
            add_characteristic(&serialCharacteristic);
        }
    };

    class SerialAdvertise : public GattAdvertise
    {
    public:
        SerialAdvertise() : GattAdvertise()
        {
        }
        
        

        GattAdvertiseFlags get_flags() override
        {
            return GattAdvertiseFlags::LE_GENERAL_DISCOVERY_MODE | GattAdvertiseFlags::BRE_NOT_SUPPORTED;
        }

        const char *get_name() override
        {
            return "Speed WATER Config";
        }

        void configure_fields(ble_hs_adv_fields &fields) override
        {
            ble_uuid16_t uuids[] = {BLE_UUID16_INIT(GATT_SVR_SVC_ALERT_UUID)};
            fields.uuids16 = uuids;
            fields.num_uuids16= 1;
            fields.uuids16_is_complete = 1;
        }
    };
} // namespace Speed::BLE::Serial
#endif // __SERIAL_BLE_H__
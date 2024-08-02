#include "mqtt/speed_mqtt.hpp"
#include "mqtt/speed_mqtt_handlers.hpp"
#include "ble/speed_ble.hpp"
#include "ble/blufi/speed_blufi_ble_addon.hpp"
#include <memory>
#include <esp_log.h>
#include "esp_system.h"
#include "settings/speed_settings.hpp"
constexpr char *TAG = "SPEED_FRAMEWORK";
using namespace Speed::Settings;
using namespace Speed::BLE;
using namespace Speed::MQTT;
using namespace Speed::BLE::Blufi;
static int makeSendText(char *buf, const char *v1, const char *v2, const char *v3, const char *v4)
{
    char DEL = 0x04;
    sprintf(buf, "%s%c%s%c%s%c%s", v1, DEL, v2, DEL, v3, DEL, v4);
    ESP_LOGD(TAG, "buf=[%s]", buf);
    return strlen(buf);
}

static void time_task(void *pvParameters)
{
    for (;;)
    {

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

extern "C" void app_main()
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    SpeedSettings::setup();

    static auto speed_mqtt = SpeedMQTT();

    auto speed_ble = SpeedBLE::setup();
    speed_ble->add_addon(SpeedBlufiAddon::setup());
    speed_ble->start("MQTT Demo");

    static const auto restart_handler = new Speed::MQTT::RestartMQTTHandler("device/1/restart");
    speed_mqtt.add_handler(restart_handler);

    while (1)
    {
        vTaskDelay(2000);
    }
}
#pragma once
#ifndef __SPEEDMQTTHANDLER_H__
#define __SPEEDMQTTHANDLER_H__
#include "speed_mqtt.hpp"
namespace Speed::MQTT
{

    class SpeedMQTTHandler
    {
    public:
        std::string *topic;
        mqtt_qos_level qos;
        int subscription_id = -1;
        handler_subscription_mode subscription_mode = handler_subscription_mode::OnStart;
        SpeedMQTT *driver;

    protected:
        explicit SpeedMQTTHandler(std::string &topic, mqtt_qos_level qos = mqtt_qos_level::One) : topic(&topic), qos(qos)
        {
        }

    public:
        void set_driver(SpeedMQTT *driver)
        {
            this->driver = driver;
        }
        virtual void before_connect(esp_mqtt_event_handle_t event, void *event_arg) {};
        virtual void handle(esp_mqtt_event_handle_t event, void *event_args) {};
        virtual void connected(esp_mqtt_event_handle_t event, void *event_arg) {};
        virtual void disconnected(esp_mqtt_event_handle_t event, void *event_arg) {};
        virtual void subscribed(esp_mqtt_event_handle_t event, void *event_arg) {};
        virtual void unsubscribed(esp_mqtt_event_handle_t event, void *event_arg) {};
    };

    class RestartMQTTHandler : public SpeedMQTTHandler
    {
        const char *TAG = "SpeedMQTTHandler";

    public:
        RestartMQTTHandler(std::string topic, mqtt_qos_level qos = mqtt_qos_level::One)
            : SpeedMQTTHandler::SpeedMQTTHandler(topic, qos)
        {
        }

    public:
        void handle(esp_mqtt_event_handle_t event, void *event_args) override
        {
            ESP_LOGW(TAG, "RESTATING THE DEVICE BY THE MQTT HANDLER: %s", event->data);
            esp_restart();
        }
    };
}

#endif // __SPEEDMQTTHANDLER_H__
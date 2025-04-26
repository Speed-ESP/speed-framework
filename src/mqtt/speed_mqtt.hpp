#pragma once

#ifndef __SPEED_MQTT_H__
#define __SPEED_MQTT_H__

#if !defined(CONFIG_SPEED_FRAMEWORK_USE_MQTT)
#error "Enable modbus in Speed Framework -> Modbus using menuconfig"
#endif

#include <stdio.h>
#include <stdexcept>
#include <string>
#include <list>
#include <map>
#include <memory>
#include <ranges>
#include <stdint.h>
#include <stddef.h>
#include <string>
#include <string.h>
#include <string_view>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "freertos/message_buffer.h"
#include "esp_log.h"
#include "mqtt_client.h"

#include "esp_system.h"
namespace Speed::MQTT
{

    const std::length_error max_handlers_reached("The maximum number of handlers has been reached!");

    const static char *SPEED_MQTT_TAG = "SpeedMQTT";
    class SpeedMQTT;

    enum class mqtt_qos_level : int
    {
        Zero = 0,
        One = 1,
        Tow = 2
    };

    enum struct handler_subscription_mode
    {
        OnStart,
        OnConnect
    };

    struct speed_mqtt_message
    {
        std::string topic;
        std::string data;
        mqtt_qos_level qos = mqtt_qos_level::Zero;
        int retain = 0;
    } ;

    class MessageRequest
    {
    public:
        int message_id;
        const speed_mqtt_message *message;
        TaskHandle_t task_handle = nullptr;

        explicit MessageRequest(speed_mqtt_message const *message) : message(message)
        {
            task_handle = xTaskGetCurrentTaskHandle();
        }

        void wait_publication(TickType_t ticks_to_wait = portMAX_DELAY) const
        {
            ulTaskNotifyTake(0, ticks_to_wait);
        }

        inline void published()
        {
            if (task_handle == nullptr)
            {
                return;
            }
            xTaskNotifyGive(task_handle);
        }
    };

    using speed_mqtt_data_event = struct
    {
        speed_mqtt_message message;
    };

    class SpeedMQTTHandler;

    class SpeedMQTT
    {

    private:
        std::multimap<std::string, SpeedMQTTHandler *, std::less<>> _handlers;
        std::multimap<std::string, MessageRequest *, std::less<>> _requests;
        esp_mqtt_client_handle_t _mqtt_client;
        std::string _topic_base;
        int _max_handlers;
        int _max_topics;

    private:
        void before_connect(esp_mqtt_event_handle_t event, void *event_arg);

        void connected(esp_mqtt_event_handle_t event, void *event_arg);

        void disconnected(esp_mqtt_event_handle_t event, void *event_arg);

        void subscribed(esp_mqtt_event_handle_t event, void *event_arg);

        void unsubscribed(esp_mqtt_event_handle_t event, void *event_arg);

        void event_published(esp_mqtt_event_handle_t event, void *event_arg);

        void handle_message(esp_mqtt_event_handle_t event, void *event_arg);

        void subscribe_handlers(handler_subscription_mode mode);

        auto get_topic_handlers(const std::string &topic)
        {
            return _handlers.find(topic);
        }

    public:
        std::string_view get_topic_base()
        {
            return _topic_base;
        }

        std::string_view combine_topic(std::string_view handler_topic) const
        {
            if (_topic_base.empty())
            {
                return handler_topic;
            }

            return _topic_base.back() + "/" + handler_topic.back();
        }

        explicit SpeedMQTT(std::string topic_base = "", int max_handlers = 20, int max_topics = 20)
            : _topic_base(topic_base),
              _max_handlers(max_handlers),
              _max_topics(max_topics)
        {
        }

        void add_handler(SpeedMQTTHandler *handler);

        void start(esp_mqtt_client_config_t *config)
        {
            _mqtt_client = esp_mqtt_client_init(config);
            /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
            ESP_ERROR_CHECK(
                esp_mqtt_client_register_event(
                    _mqtt_client,
                    MQTT_EVENT_ANY,
                    SpeedMQTT::esp_event_handler,
                    this));

            ESP_ERROR_CHECK(esp_mqtt_client_start(_mqtt_client));

            subscribe_handlers(handler_subscription_mode::OnStart);
        }

        std::unique_ptr<MessageRequest> publish_message(speed_mqtt_message *message)
        {
            auto request = std::make_unique<MessageRequest>(message);
            auto pair = std::make_pair(message->topic, request.get());
            _requests.insert(pair);
            int message_id = esp_mqtt_client_publish(
                _mqtt_client,
                message->topic.c_str(),
                message->data.c_str(),
                message->data.length(),
                (int)message->qos,
                message->retain);
            if (message_id == -1)
            {
                for (auto [i, end] = _requests.equal_range(pair.first); i != end; ++i)
                {
                    if (i->second == pair.second)
                    {
                        _requests.erase(i);
                    }
                }
            }
            request->message_id = message_id;
            return request;
        }

    private:
        void static esp_event_handler(void *args, esp_event_base_t base, int32_t event_id, void *event_data)
        {
            ESP_LOGD(SPEED_MQTT_TAG, "Event dispatched from event loop base=%s, event_id=%ld", base, event_id);
            auto *speed_mqtt = (SpeedMQTT *)args;
            esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

            switch ((esp_mqtt_event_id_t)event_id)
            {
            case MQTT_EVENT_CONNECTED:
                speed_mqtt->connected(event, event_data);
                break;
            case MQTT_EVENT_DISCONNECTED:
                speed_mqtt->disconnected(event, event_data);
                break;
            case MQTT_EVENT_SUBSCRIBED:
                speed_mqtt->subscribed(event, event_data);
                break;
            case MQTT_EVENT_UNSUBSCRIBED:
                speed_mqtt->unsubscribed(event, event_data);
                break;
            case MQTT_EVENT_PUBLISHED:
                speed_mqtt->event_published(event, event_data);
                break;
            case MQTT_EVENT_DATA:
                speed_mqtt->handle_message(event, event_data);
                break;
            case MQTT_EVENT_ERROR:
                ESP_LOGI(SPEED_MQTT_TAG, "MQTT_EVENT_ERROR");
                break;
            case MQTT_EVENT_BEFORE_CONNECT:
                speed_mqtt->before_connect(event, event_data);
                break;
            default:
                ESP_LOGI(SPEED_MQTT_TAG, "Other event id:%d", event->event_id);
                break;
            }
        };
    };

};

#endif // __SPEED_MQTT_H__
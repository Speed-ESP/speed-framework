#include "speed_mqtt.hpp"
#include "speed_mqtt_handlers.hpp"
#include "mqtt_client.h"

namespace Speed::MQTT
{

    void SpeedMQTT::before_connect(esp_mqtt_event_handle_t event, void *event_arg)
    {
        ESP_LOGI(SPEED_MQTT_TAG, "MQTT_EVENT_BEFORE_CONNECT");
        for (auto [i, end] = _handlers.equal_range(std::string(event->topic)); i != end; ++i)
        {
            i->second->before_connect(event, event_arg);
        }
    }

    void SpeedMQTT::connected(esp_mqtt_event_handle_t event, void *event_arg)
    {
        ESP_LOGI(SPEED_MQTT_TAG, "MQTT_EVENT_CONNECTED");
        subscribe_handlers(handler_subscription_mode::OnConnect);
        for (auto [i, end] = _handlers.equal_range(std::string(event->topic)); i != end; ++i)
        {
            i->second->connected(event, event_arg);
        }
    }

    void SpeedMQTT::disconnected(esp_mqtt_event_handle_t event, void *event_arg)
    {
        ESP_LOGI(SPEED_MQTT_TAG, "MQTT_EVENT_DISCONNECTED");
        for (auto [i, end] = _handlers.equal_range(std::string(event->topic)); i != end; ++i)
        {
            i->second->disconnected(event, event_arg);
        }
    }

    void SpeedMQTT::subscribed(esp_mqtt_event_handle_t event, void *event_arg)
    {
        ESP_LOGI(SPEED_MQTT_TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        for (auto [i, end] = _handlers.equal_range(std::string(event->topic)); i != end; ++i)
        {
            if (i->second->subscription_id != event->msg_id)
            {
                continue;
            }
            i->second->subscribed(event, event_arg);
            break;
        }
    }

    void SpeedMQTT::unsubscribed(esp_mqtt_event_handle_t event, void *event_arg)
    {
        ESP_LOGI(SPEED_MQTT_TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        for (auto [i, end] = _handlers.equal_range(std::string(event->topic)); i != end; ++i)
        {
            if (i->second->subscription_id != event->msg_id)
            {
                continue;
            }
            i->second->unsubscribed(event, event_arg);
            break;
        }
    }

    void SpeedMQTT::event_published(esp_mqtt_event_handle_t event, void *event_arg)
    {
        ESP_LOGI(SPEED_MQTT_TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        for (const auto &[event_id, request] : _requests)
        {
            if (request->message_id == event->msg_id)
            {
                request->published();
            }
        }
    }

    void SpeedMQTT::handle_message(esp_mqtt_event_handle_t event, void *event_arg)
    {
        ESP_LOGI(SPEED_MQTT_TAG, "MQTT_EVENT_DATA");
        for (auto [i, end] = _handlers.equal_range(std::string(event->topic)); i != end; ++i)
        {
            if (strcmp(i->first.c_str(), event->topic))
            {
                i->second->handle(event, event_arg);
            }
        }
    }

    void SpeedMQTT::subscribe_handlers(handler_subscription_mode mode)
    {
        for (const auto &[topic, handler] : _handlers)
        {
            if (handler->subscription_mode != mode)
            {
                continue;
            }
            const auto combined_topic = combine_topic(topic);

            handler->subscription_id = esp_mqtt_client_subscribe(_mqtt_client,
                                                                 combined_topic.cbegin(),
                                                                 (int)handler->qos);
        }
    }

    void SpeedMQTT::add_handler(SpeedMQTTHandler *handler)
    {
        handler->set_driver(this);
        if (_handlers.count(*handler->topic) >= _max_handlers)
        {
            throw max_handlers_reached;
        }
        _handlers.max_size();
        _handlers.insert(std::make_pair(*handler->topic, handler));
    }
}
#pragma once

#ifndef __SPEED_NET_H__
#define __SPEED_NET_H__
#include <iostream>
#include <format>
#include <string>
#include <functional>
#include <vector>
#include <memory>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <mutex>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/event_groups.h>
#include <esp_netif.h>
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"

#include <inttypes.h>

#include <SmartEnumCpp/SmartEnum.hpp>

#include <core/singleton_service.hpp>
#include <net/speed_net_def.hpp>
#include <settings/speed_settings.hpp>

namespace speed::net
{
    using namespace speed::core;

    // Forward declaration
    class SpeedWifiEventHandler;

    /**
     * @brief Core networking service that handles ESP-IDF network interfaces
     *
     * SpeedNet is responsible for initializing the ESP-IDF network stack.
     * It's a prerequisite for all networking services like WiFi and should
     * be initialized before using any networking functionality.
     *
     * Usage example:
     * ```cpp
     * // Initialize networking services
     * auto& net = Speed::Net::SpeedNet::setup();
     * ```
     */
    class SpeedNet : SingletonService<SpeedNet>
    {
    private:
        inline static const char *TAG = "SpeedNet";

    protected:
        /**
         * @brief Initialize the networking stack
         *
         * This method is called automatically the first time SpeedNet::get() is invoked.
         * It initializes the ESP-IDF network interface and event loop.
         */
        void init() override
        {
            speed::settings::SpeedSettings::setup();
            ESP_ERROR_CHECK(esp_netif_init());
            ESP_ERROR_CHECK(esp_event_loop_create_default());
            ESP_LOGI(TAG, "SpeedNet initialized");
        }

    public:
        /**
         * @brief Set up and get the SpeedNet instance
         *
         * @return SpeedNet& Reference to the singleton instance
         */
        static SpeedNet &setup()
        {
            return get();
        }
    };

    /**
     * @brief Event handler interface for WiFi events
     *
     * This class defines callback methods for various WiFi events.
     * Implement this interface to handle WiFi events such as connection,
     * disconnection, AP start/stop, etc.
     *
     * Example usage:
     * ```cpp
     * class MyWifiHandler : public SpeedWifiEventHandler {
     *   public:
     *     void onStaConnected() override {
     *         // Handle WiFi connection event
     *     }
     *
     *     void onStaDisconnected(wifi_event_sta_disconnected_t* event) override {
     *         // Handle WiFi disconnection
     *     }
     * };
     *
     * // Register the handler
     * auto handler = std::make_shared<MyWifiHandler>();
     * SpeedWifi::setup().addEventHandler(handler);
     * ```
     */
    class SpeedWifiEventHandler
    {
    public:
        /**
         * @brief Virtual destructor
         */
        virtual ~SpeedWifiEventHandler() = default;

        /**
         * @brief Called when WiFi is ready
         */
        virtual void onWifiReady() {}

        /**
         * @brief Called when WiFi scan is complete
         * @param apCount Number of access points found
         * @param list Array of access point records
         */
        virtual void onScanDone(uint16_t apCount, wifi_ap_record_t *list) {}

        /**
         * @brief Called when station mode is started
         */
        virtual void onStaStart() {}

        /**
         * @brief Called when station mode is stopped
         */
        virtual void onStaStop() {}

        /**
         * @brief Called when connected to an access point
         */
        virtual void onStaConnected() {}

        /**
         * @brief Called when disconnected from an access point
         * @param event Disconnection event data
         */
        virtual void onStaDisconnected(wifi_event_sta_disconnected_t *event) {}

        /**
         * @brief Called when an IP address is obtained
         * @param event IP event data
         */
        virtual void onStaGotIp(ip_event_got_ip_t *event) {}

        /**
         * @brief Called when AP mode is started
         */
        virtual void onApStart() {}

        /**
         * @brief Called when AP mode is stopped
         */
        virtual void onApStop() {}

        /**
         * @brief Called when a station connects to our AP
         * @param event Station connection event data
         */
        virtual void onApStaConnected(wifi_event_ap_staconnected_t *event) {}

        /**
         * @brief Called when a station disconnects from our AP
         * @param event Station disconnection event data
         */
        virtual void onApStaDisconnected(wifi_event_ap_stadisconnected_t *event) {}
    };

    /**
     * @brief WiFi management service for ESP32
     *
     * SpeedWifi provides an object-oriented interface for managing WiFi connections
     * on ESP32 devices. It supports both station (client) and access point modes,
     * and provides event-driven notifications for WiFi events.
     *
     * Example usage:
     * ```cpp
     * // Initialize and configure WiFi
     * auto& wifi = Speed::Net::SpeedWifi::setup();
     *
     * // Configure as station
     * wifi.setMode(Speed::Net::SpeedWifiMode::STA)
     *     .configureStation("MyNetwork", "MyPassword")
     *     .connect();
     * ```
     */
    class SpeedWifi : public SingletonService<SpeedWifi>
    {
    private:
        inline static wifi_init_config_t _config = WIFI_INIT_CONFIG_DEFAULT();
        inline static const char *TAG = "SpeedWifi";

        // Connection event bits
        static constexpr uint32_t WIFI_CONNECTED_BIT = BIT0;
        static constexpr uint32_t WIFI_GOT_IP_BIT = BIT1;
        static constexpr uint32_t WIFI_DISCONNECTED_BIT = BIT2;
        static constexpr uint32_t WIFI_STOP_BIT = BIT3;

        // Combined bit for full connection
        static constexpr uint32_t WIFI_CONNECTED_WITH_IP = (WIFI_CONNECTED_BIT | WIFI_GOT_IP_BIT);

        esp_netif_t *_sta_netif = nullptr;
        esp_netif_t *_ap_netif = nullptr;
        bool _event_handler_registered = false;
        std::vector<std::shared_ptr<SpeedWifiEventHandler>> _event_handlers;

        // Event group for connection synchronization
        EventGroupHandle_t _connection_event_group = nullptr;

        /**
         * @brief WiFi event handler callback
         *
         * Static callback function that processes WiFi events from ESP-IDF
         * and dispatches them to registered event handlers.
         *
         * @param arg Pointer to the SpeedWifi instance
         * @param event_base Event base (WIFI_EVENT or IP_EVENT)
         * @param event_id Event ID
         * @param event_data Event data
         */
        static void wifiEventHandler(void *arg, esp_event_base_t event_base,
                                     int32_t event_id, void *event_data);

    protected:
        /**
         * @brief Initialize the WiFi service
         *
         * This method is called automatically the first time SpeedWifi::get() is invoked.
         * It initializes the ESP-IDF WiFi driver and sets up network interfaces and event handlers.
         */
        void init() override
        {
            ESP_LOGI(TAG, "Initializing SpeedWifi");

            // Ensure network interface is initialized first
            SpeedNet::setup();

            // Initialize WiFi with the configuration
            ESP_ERROR_CHECK(esp_wifi_init(&_config));

            // Create default network interfaces
            _sta_netif = esp_netif_create_default_wifi_sta();
            _ap_netif = esp_netif_create_default_wifi_ap();

            // Create connection event group
            _connection_event_group = xEventGroupCreate();
            if (_connection_event_group == nullptr)
            {
                ESP_LOGE(TAG, "Failed to create connection event group");
            }

            // Register event handlers
            registerEventHandlers();

            ESP_LOGI(TAG, "SpeedWifi initialized successfully");
        }

        /**
         * @brief Register the WiFi event handlers with ESP-IDF
         *
         * This registers callbacks for WiFi and IP events with the ESP-IDF event system.
         */
        void registerEventHandlers()
        {
            if (!_event_handler_registered)
            {
                ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                           &SpeedWifi::wifiEventHandler, this));
                ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                           &SpeedWifi::wifiEventHandler, this));
                _event_handler_registered = true;
                ESP_LOGI(TAG, "WiFi event handlers registered");
            }
        }

    public:
        /**
         * @brief Set up and get the SpeedWifi instance
         *
         * @return SpeedWifi& Reference to the singleton instance
         */
        static SpeedWifi &setup()
        {
            return get();
        }

        /**
         * @brief Start the WiFi driver
         *
         * This starts the ESP32 WiFi driver with the current configuration.
         * Must be called after setting the mode and configuring the WiFi parameters.
         */
        void start()
        {
            ESP_ERROR_CHECK(esp_wifi_start());
            ESP_LOGI(TAG, "WiFi started");
        }

        /**
         * @brief Stop the WiFi driver
         *
         * This stops the ESP32 WiFi driver and terminates any active connections.
         */
        void stop()
        {
            ESP_ERROR_CHECK(esp_wifi_stop());
            ESP_LOGI(TAG, "WiFi stopped");
        }

        /**
         * @brief Set the WiFi operating mode
         *
         * @param mode The WiFi mode to use (station, AP, or both)
         * @return SpeedWifi& Reference to this instance for method chaining
         * @throws std::runtime_error if setting the mode fails
         */
        SpeedWifi &setMode(const SpeedWifiMode &mode)
        {
            auto result = esp_wifi_set_mode(mode);
            if (result != ESP_OK)
            {
                throw std::runtime_error("Error while configuring WiFi mode, IDF-ERROR: " +
                                         std::string(esp_err_to_name(result)));
            }
            ESP_LOGI(TAG, "WiFi mode set to %s", mode.Name().c_str());
            return *this;
        }

        /**
         * @brief Configure the WiFi station (client mode)
         *
         * @param ssid The SSID of the network to connect to
         * @param password The password for the network
         * @param useWPS Whether to use WPS for connection
         * @param fastConnect Whether to use fast connect mode
         * @return SpeedWifi& Reference to this instance for method chaining
         */
        SpeedWifi &configureStation(const std::string &ssid, const std::string &password,
                                    bool useWPS = false, bool fastConnect = true)
        {
            wifi_config_t wifi_config = {};

            // Copy SSID and password to configuration
            size_t ssid_length = std::min<size_t>(sizeof(wifi_config.sta.ssid) - 1, ssid.length());
            size_t pwd_length = std::min<size_t>(sizeof(wifi_config.sta.password) - 1, password.length());

            memcpy(wifi_config.sta.ssid, ssid.c_str(), ssid_length);
            memcpy(wifi_config.sta.password, password.c_str(), pwd_length);

            // Use the SmartEnum classes for better type safety
            wifi_config.sta.scan_method = useWPS ? SpeedWifiScanMethod::FAST : SpeedWifiScanMethod::ALL_CHANNEL;
            wifi_config.sta.bssid_set = false;
            wifi_config.sta.channel = 0;
            wifi_config.sta.listen_interval = 0;
            wifi_config.sta.sort_method = SpeedWifiSortMethod::BY_RSSI;

            if (fastConnect && !password.empty())
            {
                wifi_config.sta.threshold.authmode = SpeedWifiAuthMode::WPA2_PSK;
            }
            else
            {
                wifi_config.sta.threshold.authmode = SpeedWifiAuthMode::OPEN;
            }

            wifi_config.sta.pmf_cfg.capable = true;
            wifi_config.sta.pmf_cfg.required = false;

            ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
            ESP_LOGI(TAG, "WiFi station configured with SSID: %s", ssid.c_str());

            return *this;
        }

        /**
         * @brief Configure the WiFi access point (AP mode)
         *
         * @param ssid The SSID for the access point
         * @param password The password for the access point
         * @param authMode Authentication mode to use
         * @param channel WiFi channel to use (1-13)
         * @param maxConnections Maximum number of stations that can connect
         * @param hidden Whether to hide the SSID
         * @return SpeedWifi& Reference to this instance for method chaining
         */
        SpeedWifi &configureAccessPoint(const std::string &ssid, const std::string &password,
                                        const SpeedWifiAuthMode &authMode = SpeedWifiAuthMode::WPA2_PSK,
                                        uint8_t channel = 1, uint8_t maxConnections = 4,
                                        bool hidden = false)
        {
            wifi_config_t wifi_config = {};

            // Copy SSID and password to configuration
            size_t ssid_length = std::min<size_t>(sizeof(wifi_config.ap.ssid) - 1, ssid.length());
            size_t pwd_length = std::min<size_t>(sizeof(wifi_config.ap.password) - 1, password.length());

            memcpy(wifi_config.ap.ssid, ssid.c_str(), ssid_length);
            wifi_config.ap.ssid_len = ssid.length();
            memcpy(wifi_config.ap.password, password.c_str(), pwd_length);

            wifi_config.ap.channel = channel;
            wifi_config.ap.max_connection = maxConnections;
            wifi_config.ap.authmode = authMode;
            wifi_config.ap.ssid_hidden = hidden ? 1 : 0;

            // Set open authentication if no password
            if (password.empty())
            {
                wifi_config.ap.authmode = SpeedWifiAuthMode::OPEN;
            }

            ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
            ESP_LOGI(TAG, "WiFi access point configured with SSID: %s, channel: %d, auth mode: %s",
                     ssid.c_str(), channel, authMode.Name().c_str());

            return *this;
        }

        /**
         * @brief Connect to the configured WiFi network in station mode
         *
         * This attempts to connect to the WiFi network configured with configureStation().
         * The mode must be set to STA or APSTA, and the station must be configured.
         */
        void connect()
        {
            ESP_ERROR_CHECK(esp_wifi_connect());
            ESP_LOGI(TAG, "Connecting to WiFi network...");
        }

        /**
         * @brief Disconnect from the current WiFi network in station mode
         *
         * This disconnects from any connected WiFi network in station mode.
         */
        void disconnect()
        {
            ESP_ERROR_CHECK(esp_wifi_disconnect());
            ESP_LOGI(TAG, "Disconnected from WiFi network");
        }

        /**
         * @brief Start a WiFi network scan
         *
         * @param blockingMode Whether to wait for scan completion
         * @param showHidden Whether to show hidden networks
         * @param scanType Scan type (active or passive)
         * @param timeout_ms Timeout in milliseconds (0 for default)
         */
        void startScan(bool blockingMode = false, bool showHidden = false,
                       const SpeedWifiScanType &scanType = SpeedWifiScanType::ACTIVE,
                       uint32_t timeout_ms = 0)
        {
            wifi_scan_config_t scan_config = {};
            scan_config.ssid = nullptr;
            scan_config.bssid = nullptr;
            scan_config.channel = 0;
            scan_config.show_hidden = showHidden;
            scan_config.scan_type = scanType;
            scan_config.scan_time.active.min = 100;
            scan_config.scan_time.active.max = 300;
            scan_config.scan_time.passive = 500;

            ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, blockingMode));
            ESP_LOGI(TAG, "WiFi scan started with scan type: %s", scanType.Name().c_str());
        }

        /**
         * @brief Get the results of the last WiFi scan
         *
         * This method should be called after a scan is complete (wait for
         * the onScanDone callback or use blocking mode in startScan).
         *
         * @return std::vector<wifi_ap_record_t> Vector of access point records
         */
        std::vector<wifi_ap_record_t> getScanResults()
        {
            uint16_t ap_count = 0;
            ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));

            if (ap_count == 0)
            {
                ESP_LOGI(TAG, "No access points found");
                return {};
            }

            std::vector<wifi_ap_record_t> ap_records(ap_count);
            ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, ap_records.data()));
            ESP_LOGI(TAG, "Found %d access points", ap_count);

            return ap_records;
        }

        /**
         * @brief Add an event handler for WiFi events
         *
         * @param handler Shared pointer to a SpeedWifiEventHandler implementation
         */
        void addEventHandler(std::shared_ptr<SpeedWifiEventHandler> handler)
        {
            // Use the base class thread-safe method
            withInit([this, handler]()
                     {
                _event_handlers.push_back(handler);
                ESP_LOGI(TAG, "Added WiFi event handler"); });
        }

        /**
         * @brief Get information about the currently connected access point
         *
         * @return wifi_ap_record_t Information about the connected AP
         * @throws ESP_ERR_WIFI_NOT_CONNECT if not connected
         */
        wifi_ap_record_t getConnectionInfo()
        {
            wifi_ap_record_t ap_info = {};
            ESP_ERROR_CHECK(esp_wifi_sta_get_ap_info(&ap_info));
            return ap_info;
        }

        /**
         * @brief Wait for a WiFi connection to be established
         *
         * This method blocks until a connection is established or the timeout is reached.
         * Uses an event group for efficient waiting instead of polling.
         *
         * @param timeout_ms Maximum time to wait in milliseconds
         * @return bool True if connected, false if timed out
         */
        bool waitForConnection(uint32_t timeout_ms = 10000)
        {
            if (_connection_event_group == nullptr)
            {
                ESP_LOGE(TAG, "Connection event group is null");
                return false;
            }

            // If already connected and have IP, return immediately
            EventBits_t bits = xEventGroupGetBits(_connection_event_group);
            if ((bits & WIFI_CONNECTED_WITH_IP) == WIFI_CONNECTED_WITH_IP)
            {
                return true;
            }

            // Calculate ticks to wait
            TickType_t xTicksToWait = pdMS_TO_TICKS(timeout_ms);

            // Wait for the event bits to be set by the event handler
            ESP_LOGI(TAG, "Waiting for WiFi connection (timeout:%" PRIu32 " ms)", timeout_ms);
            bits = xEventGroupWaitBits(_connection_event_group, WIFI_CONNECTED_WITH_IP,
                                       pdFALSE, pdTRUE, xTicksToWait);

            if ((bits & WIFI_CONNECTED_WITH_IP) == WIFI_CONNECTED_WITH_IP)
            {
                ESP_LOGI(TAG, "WiFi connection established successfully");
                return true;
            }
            else
            {
                ESP_LOGW(TAG, "WiFi connection timeout");
                return false;
            }
        }
    };
    // Implementation of the static WiFi event handler
    inline void SpeedWifi::wifiEventHandler(void *arg, esp_event_base_t event_base,
                                            int32_t event_id, void *event_data)
    {
        SpeedWifi *wifi = static_cast<SpeedWifi *>(arg);

        if (event_base == WIFI_EVENT)
        {
            switch (event_id)
            {
            case WIFI_EVENT_WIFI_READY:
                ESP_LOGI(TAG, "WiFi ready");
                for (auto &handler : wifi->_event_handlers)
                {
                    handler->onWifiReady();
                }
                break;

            case WIFI_EVENT_SCAN_DONE:
            {
                uint16_t ap_count = 0;
                ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));

                if (ap_count > 0)
                {
                    std::vector<wifi_ap_record_t> ap_records(ap_count);
                    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, ap_records.data()));
                    ESP_LOGI(TAG, "Scan done: found %d access points", ap_count);

                    for (auto &handler : wifi->_event_handlers)
                    {
                        handler->onScanDone(ap_count, ap_records.data());
                    }
                }
                else
                {
                    ESP_LOGI(TAG, "Scan done: no access points found");
                    for (auto &handler : wifi->_event_handlers)
                    {
                        handler->onScanDone(0, nullptr);
                    }
                }
                break;
            }

            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "WiFi station started");
                for (auto &handler : wifi->_event_handlers)
                {
                    handler->onStaStart();
                }
                break;

            case WIFI_EVENT_STA_STOP:
                ESP_LOGI(TAG, "WiFi station stopped");
                xEventGroupSetBits(wifi->_connection_event_group, WIFI_STOP_BIT);
                xEventGroupClearBits(wifi->_connection_event_group, WIFI_CONNECTED_BIT | WIFI_GOT_IP_BIT);
                for (auto &handler : wifi->_event_handlers)
                {
                    handler->onStaStop();
                }
                break;

            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "WiFi connected to AP");
                xEventGroupSetBits(wifi->_connection_event_group, WIFI_CONNECTED_BIT);
                for (auto &handler : wifi->_event_handlers)
                {
                    handler->onStaConnected();
                }
                break;

            case WIFI_EVENT_STA_DISCONNECTED:
            {
                auto *event = static_cast<wifi_event_sta_disconnected_t *>(event_data);
                // Find the disconnect reason enum by its value
                auto reason = SpeedWifiDisconnectReason::FromValue(event->reason);
                ESP_LOGW(TAG, "WiFi disconnected from AP, reason: %s (%d)",
                         reason->getDescription().c_str(), event->reason);

                xEventGroupSetBits(wifi->_connection_event_group, WIFI_DISCONNECTED_BIT);
                xEventGroupClearBits(wifi->_connection_event_group, WIFI_CONNECTED_BIT | WIFI_GOT_IP_BIT);

                for (auto &handler : wifi->_event_handlers)
                {
                    handler->onStaDisconnected(event);
                }
                break;
            }

            case WIFI_EVENT_AP_START:
                ESP_LOGI(TAG, "WiFi access point started");
                for (auto &handler : wifi->_event_handlers)
                {
                    handler->onApStart();
                }
                break;

            case WIFI_EVENT_AP_STOP:
                ESP_LOGI(TAG, "WiFi access point stopped");
                for (auto &handler : wifi->_event_handlers)
                {
                    handler->onApStop();
                }
                break;

            case WIFI_EVENT_AP_STACONNECTED:
            {
                auto *event = static_cast<wifi_event_ap_staconnected_t *>(event_data);
                char mac_str[18] = {0};
                snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
                         event->mac[0], event->mac[1], event->mac[2],
                         event->mac[3], event->mac[4], event->mac[5]);
                ESP_LOGI(TAG, "Station %s joined, AID=%d", mac_str, event->aid);
                for (auto &handler : wifi->_event_handlers)
                {
                    handler->onApStaConnected(event);
                }
                break;
            }

            case WIFI_EVENT_AP_STADISCONNECTED:
            {
                auto *event = static_cast<wifi_event_ap_stadisconnected_t *>(event_data);
                char mac_str[18] = {0};
                snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
                         event->mac[0], event->mac[1], event->mac[2],
                         event->mac[3], event->mac[4], event->mac[5]);
                ESP_LOGI(TAG, "Station %s left, AID=%d", mac_str, event->aid);
                for (auto &handler : wifi->_event_handlers)
                {
                    handler->onApStaDisconnected(event);
                }
                break;
            }
            }
        }
        else if (event_base == IP_EVENT)
        {
            switch (event_id)
            {
            case IP_EVENT_STA_GOT_IP:
            {
                auto *event = static_cast<ip_event_got_ip_t *>(event_data);

                char ip_buffer[16];
                esp_ip4addr_ntoa(&event->ip_info.ip, ip_buffer, sizeof(ip_buffer));
                ESP_LOGI(TAG, "Got IP address: %s", ip_buffer);
                xEventGroupSetBits(wifi->_connection_event_group, WIFI_GOT_IP_BIT);

                for (auto &handler : wifi->_event_handlers)
                {
                    handler->onStaGotIp(event);
                }
                break;
            }
            }
        }
    }
};

#endif // __SPEED_NET_H__
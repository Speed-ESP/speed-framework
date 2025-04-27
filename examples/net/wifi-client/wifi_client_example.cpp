#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <net/speed_net.hpp>
#include <memory>

using namespace speed::net;

// Configuration - Replace with your WiFi credentials
const char* WIFI_SSID = "YourWiFiNetwork";
const char* WIFI_PASSWORD = "YourWiFiPassword";
const int CONNECTION_TIMEOUT_MS = 30000; // 30 seconds
// WiFi event handler class
class WiFiClientHandler : public SpeedWifiEventHandler {
private:
    bool connected = false;
    bool has_ip = false;

public:
    // Called when connected to an access point
    void onStaConnected() override {
        connected = true;
        printf("Connected to WiFi network!\n");
    }
    
    // Called when disconnected from an access point
    void onStaDisconnected(wifi_event_sta_disconnected_t* event) override {
        connected = false;
        has_ip = false;
        
        // Get the disconnect reason using our smart enum
        SpeedWifiDisconnectReason reason = 
            SpeedWifiDisconnectReason::FromValue(event->reason);
        
        printf("Disconnected from WiFi. Reason: %s\n", reason.getDescription().c_str());
    }
    
    // Called when an IP address is obtained
    void onStaGotIp(ip_event_got_ip_t* event) override {
        has_ip = true;
        
        // Format the IP address for display
        char ip_str[16];
        snprintf(ip_str, sizeof(ip_str), "%u.%u.%u.%u", 
                (event->ip_info.ip.addr & 0xFF),
                ((event->ip_info.ip.addr >> 8) & 0xFF),
                ((event->ip_info.ip.addr >> 16) & 0xFF),
                ((event->ip_info.ip.addr >> 24) & 0xFF));
                
        printf("Got IP address: %s\n", ip_str);
    }
    
    bool isFullyConnected() const {
        return connected && has_ip;
    }
};

// Format authentication mode to string for display
const char* authModeToString(wifi_auth_mode_t mode) {
    switch (mode) {
        case WIFI_AUTH_OPEN: return "OPEN";
        case WIFI_AUTH_WEP: return "WEP";
        case WIFI_AUTH_WPA_PSK: return "WPA_PSK";
        case WIFI_AUTH_WPA2_PSK: return "WPA2_PSK";
        case WIFI_AUTH_WPA_WPA2_PSK: return "WPA_WPA2_PSK";
        case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2_ENTERPRISE";
        case WIFI_AUTH_WPA3_PSK: return "WPA3_PSK";
        case WIFI_AUTH_WPA2_WPA3_PSK: return "WPA2_WPA3_PSK";
        default: return "UNKNOWN";
    }
}

extern "C" void app_main(void)
{
    // Create and register our event handler
    auto handler = std::make_shared<WiFiClientHandler>();
    
    try {
        // Initialize and configure WiFi
        auto& wifi =SpeedWifi::setup();
        
        // Add our event handler
        wifi.addEventHandler(handler);
        
        // Configure as station (client) mode
        wifi.setMode(SpeedWifiMode::STA)
            .configureStation(WIFI_SSID, WIFI_PASSWORD)
            .start();
        
        // Connect to the network
        wifi.connect();
        
        // Wait for connection
        printf("Connecting to %s...\n", WIFI_SSID);
        
        // Wait for a full connection (up to 30 seconds)
        uint32_t start_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
        while (!handler->isFullyConnected()) {
            vTaskDelay(pdMS_TO_TICKS(100));
            
            // Check for timeout
            uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
            if ((current_time - start_time) > CONNECTION_TIMEOUT_MS) {
                printf("Connection timeout!\n");
                break;
            }
        }
        
        // If we're connected, display connection information
        if (handler->isFullyConnected()) {
            printf("Connected successfully!\n");
            
            // Get info about the connected AP
            wifi_ap_record_t ap_info = wifi.getConnectionInfo();
            
            printf("Connection information:\n");
            printf("  SSID: %s\n", ap_info.ssid);
            printf("  Channel: %d\n", ap_info.primary);
            printf("  RSSI: %d\n", ap_info.rssi);
            printf("  Authentication Mode: %s\n", authModeToString(ap_info.authmode));
        }
        
        // Keep the program running
        printf("\nExample complete. Device will remain connected.\n");
        while (true) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        
    } catch (const std::exception& e) {
        printf("Error: %s\n", e.what());
    }
}
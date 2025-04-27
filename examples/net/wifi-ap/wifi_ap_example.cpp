#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <net/speed_net.hpp>
#include <memory>

using namespace speed::net;

// Configuration for the access point
const char* AP_SSID = "ESP32-AP";
const char* AP_PASSWORD = "password123";
const uint8_t AP_CHANNEL = 6;
const uint8_t AP_MAX_CONNECTIONS = 4;
const bool AP_SSID_HIDDEN = false;

// Helper function to format MAC address to string
void formatMacAddress(const uint8_t* mac, char* output) {
    snprintf(output, 18, "%02X:%02X:%02X:%02X:%02X:%02X", 
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

// WiFi event handler for access point mode
class APEventHandler : public speed::net::SpeedWifiEventHandler {
public:
    // Called when a station connects to our AP
    void onApStaConnected(wifi_event_ap_staconnected_t* event) override {
        char mac_str[18] = {0};
        formatMacAddress(event->mac, mac_str);
        
        printf("Station connected!\n");
        printf("  MAC: %s\n", mac_str);
        printf("  AID: %d\n", event->aid);
    }
    
    // Called when a station disconnects from our AP
    void onApStaDisconnected(wifi_event_ap_stadisconnected_t* event) override {
        char mac_str[18] = {0};
        formatMacAddress(event->mac, mac_str);
        
        printf("Station disconnected!\n");
        printf("  MAC: %s\n", mac_str);
        printf("  AID: %d\n", event->aid);
    }
};

extern "C" void app_main(void)
{
    // Create our event handler
    auto handler = std::make_shared<APEventHandler>();
    
    try {
        // Initialize and configure WiFi
        auto& wifi = speed::net::SpeedWifi::setup();
        
        // Register our event handler
        wifi.addEventHandler(handler);
        
        // Configure as access point
        wifi.setMode(speed::net::SpeedWifiMode::AP)
            .configureAccessPoint(
                AP_SSID, 
                AP_PASSWORD, 
                speed::net::SpeedWifiAuthMode::WPA2_PSK,
                AP_CHANNEL,
                AP_MAX_CONNECTIONS,
                AP_SSID_HIDDEN
            )
            .start();
        
        // Print access point information
        printf("Access point started!\n");
        printf("  SSID: %s\n", AP_SSID);
        printf("  Password: %s\n", AP_PASSWORD);
        printf("  IP Address: 192.168.4.1\n");  // Default AP IP
        
        printf("\nWaiting for stations to connect...\n\n");
        
        // Keep the program running
        while (true) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        
    } catch (const std::exception& e) {
        printf("Error: %s\n", e.what());
    }
}
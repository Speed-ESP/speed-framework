#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <net/speed_net.hpp>
#include <memory>

using namespace speed::net;

// Event group to signal WiFi connection status
static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;
const int GOT_IP_BIT = BIT1;

// Configuration - Replace with your WiFi credentials
const char* STA_SSID = "YourWiFiNetwork";
const char* STA_PASSWORD = "YourWiFiPassword";

// Access point configuration
const char* AP_SSID = "ESP32-DualMode";
const char* AP_PASSWORD = "password123";
const uint8_t AP_CHANNEL = 6;
const uint8_t AP_MAX_CONNECTIONS = 4;
const bool AP_SSID_HIDDEN = false;

// Helper function to format MAC address to string
void formatMacAddress(const uint8_t* mac, char* output) {
    snprintf(output, 18, "%02X:%02X:%02X:%02X:%02X:%02X", 
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

// WiFi event handler for dual mode
class DualModeHandler : public SpeedWifiEventHandler {
private:
    bool sta_connected = false;
    bool has_ip = false;
    int ap_stations = 0;
    
public:
    // Called when station connects to AP
    void onStaConnected() override {
        sta_connected = true;
        printf("Station connected to network!\n\n");
        
        // Set the event bit
        if (wifi_event_group != NULL) {
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        }
    }
    
    // Called when station disconnects from AP
    void onStaDisconnected(wifi_event_sta_disconnected_t* event) override {
        sta_connected = false;
        has_ip = false;
        
        // Get the disconnect reason using our smart enum
        SpeedWifiDisconnectReason reason = 
            SpeedWifiDisconnectReason::FromValue(event->reason);
        
        printf("Station disconnected. Reason: %s\n", reason.getDescription().c_str());
        
        // Clear the connection bits
        if (wifi_event_group != NULL) {
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT | GOT_IP_BIT);
        }
    }
    
    // Called when station gets IP address
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
        printf("Station connection complete!\n\n");
        
        // Set the IP bit
        if (wifi_event_group != NULL) {
            xEventGroupSetBits(wifi_event_group, GOT_IP_BIT);
        }
    }
    
    // Called when a station connects to our AP
    void onApStaConnected(wifi_event_ap_staconnected_t* event) override {
        ap_stations++;
        
        char mac_str[18] = {0};
        formatMacAddress(event->mac, mac_str);
        
        printf("Station connected to our access point!\n");
        printf("  MAC: %s\n", mac_str);
        printf("  AID: %d\n", event->aid);
        printf("  Total stations connected: %d\n\n", ap_stations);
    }
    
    // Called when a station disconnects from our AP
    void onApStaDisconnected(wifi_event_ap_stadisconnected_t* event) override {
        if (ap_stations > 0) {
            ap_stations--;
        }
        
        char mac_str[18] = {0};
        formatMacAddress(event->mac, mac_str);
        
        printf("Station disconnected from our access point!\n");
        printf("  MAC: %s\n", mac_str);
        printf("  AID: %d\n", event->aid);
        printf("  Total stations connected: %d\n\n", ap_stations);
    }
    
    bool isStaConnected() const {
        return sta_connected;
    }
    
    bool hasIp() const {
        return has_ip;
    }
    
    int getApStations() const {
        return ap_stations;
    }
};

extern "C" void app_main(void) {
    // Create event group
    wifi_event_group = xEventGroupCreate();
    
    // Create our event handler
    auto handler = std::make_shared<DualModeHandler>();
    
    try {
        // Initialize and configure WiFi
        auto& wifi = SpeedWifi::setup();
        
        // Register our event handler
        wifi.addEventHandler(handler);
        
        // Configure as dual mode (APSTA)
        wifi.setMode(SpeedWifiMode::APSTA);
        
        // Configure station (client) mode
        wifi.configureStation(STA_SSID, STA_PASSWORD);
        
        // Configure access point mode
        wifi.configureAccessPoint(
            AP_SSID, 
            AP_PASSWORD, 
            SpeedWifiAuthMode::WPA2_PSK,
            AP_CHANNEL,
            AP_MAX_CONNECTIONS,
            AP_SSID_HIDDEN
        );
        
        // Start WiFi
        wifi.start();
        
        // Connect to the WiFi network
        wifi.connect();
        
        // Print status information
        printf("\nDual Mode WiFi started!\n\n");
        printf("Station (client) mode:\n");
        printf("  Connecting to: %s\n", STA_SSID);
        printf("  Status: Connecting...\n\n");
        
        printf("Access Point mode:\n");
        printf("  SSID: %s\n", AP_SSID);
        printf("  Password: %s\n", AP_PASSWORD);
        printf("  IP Address: 192.168.4.1\n\n");  // Default AP IP
        
        // Wait for connection (optional)
        printf("Waiting for station connection...\n");
        EventBits_t bits = xEventGroupWaitBits(
            wifi_event_group,
            CONNECTED_BIT | GOT_IP_BIT,
            pdFALSE,  // Don't clear on exit
            pdTRUE,   // Wait for all bits
            portMAX_DELAY  // Wait forever
        );
        
        if ((bits & (CONNECTED_BIT | GOT_IP_BIT)) == (CONNECTED_BIT | GOT_IP_BIT)) {
            printf("Fully connected in dual mode!\n\n");
        }
        
        // Keep the program running
        printf("Dual mode example running. Press Ctrl+C to exit.\n");
        while (true) {
            // Periodically print status
            printf("Status: STA %s, AP: %d stations connected\n", 
                  handler->hasIp() ? "connected" : "disconnected",
                  handler->getApStations());
                  
            vTaskDelay(pdMS_TO_TICKS(10000)); // Status update every 10 seconds
        }
        
    } catch (const std::exception& e) {
        printf("Error: %s\n", e.what());
    }
    
    // Clean up
    if (wifi_event_group != NULL) {
        vEventGroupDelete(wifi_event_group);
    }
}
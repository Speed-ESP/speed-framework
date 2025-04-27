#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <net/speed_net.hpp>
#include <memory>

using namespace speed::net;
// Event group to signal when scan is complete
static EventGroupHandle_t wifi_event_group;
const int SCAN_DONE_BIT = BIT0;

// Function to interpret RSSI values
const char* getRssiQuality(int rssi) {
    if (rssi >= -50) {
        return "Excellent";
    } else if (rssi >= -60) {
        return "Good";
    } else if (rssi >= -70) {
        return "Fair";
    } else if (rssi >= -80) {
        return "Weak";
    } else {
        return "Poor";
    }
}

// Function to convert auth mode to string representation
const char* getAuthModeName(wifi_auth_mode_t mode) {
    switch (mode) {
        case WIFI_AUTH_OPEN:
            return "OPEN";
        case WIFI_AUTH_WEP:
            return "WEP";
        case WIFI_AUTH_WPA_PSK:
            return "WPA_PSK";
        case WIFI_AUTH_WPA2_PSK:
            return "WPA2_PSK";
        case WIFI_AUTH_WPA_WPA2_PSK:
            return "WPA_WPA2_PSK";
        case WIFI_AUTH_WPA2_ENTERPRISE:
            return "WPA2_ENTERPRISE";
        case WIFI_AUTH_WPA3_PSK:
            return "WPA3_PSK";
        case WIFI_AUTH_WPA2_WPA3_PSK:
            return "WPA2_WPA3_PSK";
        default:
            return "UNKNOWN";
    }
}

// WiFi event handler for scanning
class ScanEventHandler : public SpeedWifiEventHandler {
private:
    bool scan_complete = false;
    
public:
    // Called when WiFi scan is complete
    void onScanDone(uint16_t apCount, wifi_ap_record_t* list) override {
        printf("Scan complete! Found %d networks:\n\n", apCount);
        
        // Display information about each network
        for (int i = 0; i < apCount; i++) {
            wifi_ap_record_t ap = list[i];
            
            printf("Network %d:\n", i + 1);
            printf("  SSID: %s\n", ap.ssid);
            printf("  RSSI: %d dBm (%s)\n", ap.rssi, getRssiQuality(ap.rssi));
            printf("  Channel: %d\n", ap.primary);
            printf("  Auth Mode: %s\n", getAuthModeName(ap.authmode));
            printf("  Hidden: %s\n", ap.ssid[0] == 0 ? "Yes" : "No");
            printf("\n");
        }
        
        scan_complete = true;
        
        // Set the event bit to indicate scan is complete
        if (wifi_event_group != NULL) {
            xEventGroupSetBits(wifi_event_group, SCAN_DONE_BIT);
        }
    }
    
    bool isScanComplete() const {
        return scan_complete;
    }
};

extern "C" void app_main(void) {
    // Create event group
    wifi_event_group = xEventGroupCreate();
    
    // Create our event handler
    auto handler = std::make_shared<ScanEventHandler>();
    
    try {
        // Initialize and configure WiFi
        auto& wifi = SpeedWifi::setup();
        
        // Register our event handler
        wifi.addEventHandler(handler);
        
        // Configure as station mode for scanning
        wifi.setMode(SpeedWifiMode::STA)
            .start();
        
        printf("Starting WiFi scan...\n\n");
        
        // Start non-blocking scan, showing hidden networks, using active scan
        wifi.startScan(
            false,                               // Non-blocking mode
            true,                                // Show hidden networks
            SpeedWifiScanType::ACTIVE  // Use active scanning
        );
        
        // Wait for scan to complete
        EventBits_t bits = xEventGroupWaitBits(
            wifi_event_group,
            SCAN_DONE_BIT,
            pdTRUE,  // Clear on exit
            pdFALSE, // Don't wait for all bits
            portMAX_DELAY  // Wait forever
        );
        
        // Scan results are already displayed by the event handler
        
        // Keep the program running
        printf("Scan complete. To scan again, reset the device.\n");
        while (true) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        
    } catch (const std::exception& e) {
        printf("Error: %s\n", e.what());
    }
    
    // Clean up
    if (wifi_event_group != NULL) {
        vEventGroupDelete(wifi_event_group);
    }
}
# Network Framework Documentation

The Speed Framework provides a comprehensive networking stack for ESP32 devices, offering an object-oriented and type-safe approach to WiFi configuration and management.

## Overview

The networking framework consists of several key components:

- **SpeedNet**: Core networking service that initializes the ESP-IDF network stack
- **SpeedWifi**: WiFi management service that handles WiFi connections in both station and access point modes
- **SmartEnum Classes**: Type-safe enumeration classes for various WiFi settings and states

## Features

- Thread-safe singleton implementation for networking services
- Fluent interface for easy configuration
- Event-driven architecture for handling WiFi events
- Support for both station (client) and access point modes
- Type-safe enums for improved code reliability and readability
- Comprehensive WiFi scanning capabilities
- Full control over WiFi parameters (authentication, channels, etc.)
- Detailed logging and error reporting

## Getting Started

### Basic WiFi Client Connection

```cpp
#include <net/speed_net.hpp>

void app_main()
{
    // Initialize and configure WiFi as a station (client)
    auto& wifi = speed::net::SpeedWifi::setup();
    
    // Configure as station
    wifi.setMode(speed::net::SpeedWifiMode::STA)
        .configureStation("MyNetwork", "MyPassword")
        .start();
    
    // Connect to the configured network
    wifi.connect();
    
    // Wait for connection (optional)
    if (wifi.waitForConnection(10000)) {
        printf("Connected to WiFi successfully!\n");
        
        // Get connection info
        wifi_ap_record_t info = wifi.getConnectionInfo();
        printf("Connected to: %s, Channel: %d, RSSI: %d\n", 
               info.ssid, info.primary, info.rssi);
    } else {
        printf("Failed to connect to WiFi\n");
    }
}
```

### Create an Access Point

```cpp
#include <net/speed_net.hpp>

void app_main()
{
    // Initialize and configure WiFi as an access point
    auto& wifi = speed::net::SpeedWifi::setup();
    
    // Configure as access point
    wifi.setMode(speed::net::SpeedWifiMode::AP)
        .configureAccessPoint("MyESP32AP", "password123", 
                             speed::net::SpeedWifiAuthMode::WPA2_PSK,
                             6,  // Channel
                             4,  // Max connections
                             false)  // Not hidden
        .start();
    
    printf("Access point started!\n");
}
```

### Handle WiFi Events

```cpp
#include <net/speed_net.hpp>
#include <memory>

// Create an event handler
class MyWifiHandler : public speed::net::SpeedWifiEventHandler {
public:
    void onStaConnected() override {
        printf("Connected to WiFi network!\n");
    }
    
    void onStaDisconnected(wifi_event_sta_disconnected_t* event) override {
        printf("Disconnected from WiFi. Reason: %s\n", 
               speed::net::SpeedWifiDisconnectReason::FromValue(event->reason).getDescription().c_str());
    }
    
    void onStaGotIp(ip_event_got_ip_t* event) override {
        char ip_str[16];
        sprintf(ip_str, "%u.%u.%u.%u", 
                (event->ip_info.ip.addr & 0xFF),
                ((event->ip_info.ip.addr >> 8) & 0xFF),
                ((event->ip_info.ip.addr >> 16) & 0xFF),
                ((event->ip_info.ip.addr >> 24) & 0xFF));
        printf("Got IP address: %s\n", ip_str);
    }
};

void app_main()
{
    // Initialize WiFi
    auto& wifi = speed::net::SpeedWifi::setup();
    
    // Register event handler
    auto handler = std::make_shared<MyWifiHandler>();
    wifi.addEventHandler(handler);
    
    // Configure and start WiFi
    wifi.setMode(speed::net::SpeedWifiMode::STA)
        .configureStation("MyNetwork", "MyPassword")
        .start();
    
    // Connect
    wifi.connect();
}
```

### Scan for Networks

```cpp
#include <net/speed_net.hpp>
#include <memory>

// Create an event handler for scan results
class ScanHandler : public speed::net::SpeedWifiEventHandler {
public:
    void onScanDone(uint16_t apCount, wifi_ap_record_t* list) override {
        printf("Found %d networks:\n", apCount);
        
        for (int i = 0; i < apCount; i++) {
            printf("  %d. SSID: %s, RSSI: %d, Channel: %d, Auth: %d\n",
                   i + 1, list[i].ssid, list[i].rssi, 
                   list[i].primary, list[i].authmode);
        }
    }
};

void app_main()
{
    // Initialize WiFi
    auto& wifi = speed::net::SpeedWifi::setup();
    
    // Register scan handler
    auto handler = std::make_shared<ScanHandler>();
    wifi.addEventHandler(handler);
    
    // Configure WiFi for scanning
    wifi.setMode(speed::net::SpeedWifiMode::STA)
        .start();
    
    // Start scan (non-blocking)
    wifi.startScan(false, true, speed::net::SpeedWifiScanType::ACTIVE);
    
    // Results will be delivered to the event handler
}
```

## API Reference

For complete API documentation, refer to the [API Reference](api_reference.md).

## Advanced Usage

For more advanced usage scenarios, check out the examples in the `examples/net` directory:

- [WiFi Client Example](../../examples/net/wifi-client/README.md)
- [WiFi Access Point Example](../../examples/net/wifi-ap/README.md)
- [WiFi Scanner Example](../../examples/net/wifi-scanner/README.md)
- [Dual Mode Example](../../examples/net/wifi-dual-mode/README.md)

## Thread Safety

All networking services in Speed Framework are thread-safe. The `SingletonService` base class ensures that initialization happens only once, even in multi-threaded environments, and provides mechanisms for thread-safe access to shared resources.

## Error Handling

The framework uses two main approaches for error handling:

1. **ESP-IDF Error Codes**: Most methods pass through ESP-IDF error codes, which can be checked using `ESP_ERROR_CHECK()`.
2. **Exceptions**: Some methods, like `setMode()`, throw exceptions with detailed error messages when failures occur.

## Logging

The framework uses the ESP-IDF logging system with tags "SpeedNet" and "SpeedWifi". You can configure the log level in your project's `sdkconfig` file to control verbosity.
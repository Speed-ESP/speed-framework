# WiFi Dual Mode Example

This example demonstrates how to use the Speed Framework to configure an ESP32 device to operate in both station (client) and access point modes simultaneously.

## Features

- Connects to an existing WiFi network as a client
- Creates its own access point for other devices to connect
- Handles connection events for both modes
- Shows how to configure and manage different WiFi interfaces
- Uses SmartEnum classes for type-safe WiFi configuration

## Hardware Required

- ESP32 development board
- USB cable for programming and power
- WiFi-capable devices to test both connections

## Setup

1. Update the WiFi client credentials in `main.cpp` with your network's SSID and password
2. Modify the access point configuration if desired
3. Build and flash the example to your ESP32
4. Connect to the created access point from another device

## Code Overview

The example demonstrates:

- Setting up the WiFi service in APSTA mode (both AP and station)
- Configuring client connection parameters
- Configuring access point parameters
- Handling WiFi events for both modes
- Managing IP addresses for both interfaces

## How It Works

The example creates a custom event handler to respond to WiFi events such as station connection/disconnection and client connection status. It configures the WiFi interface in APSTA mode to operate as both a client and an access point simultaneously.

## Expected Output

If everything works correctly, you should see output similar to the following on the serial monitor:

```
I (345) SpeedNet: SpeedNet initialized
I (352) SpeedWifi: Initializing SpeedWifi
I (362) SpeedWifi: WiFi event handlers registered
I (367) SpeedWifi: SpeedWifi initialized successfully
I (374) SpeedWifi: WiFi mode set to APSTA
I (381) SpeedWifi: WiFi station configured with SSID: MyNetwork
I (387) SpeedWifi: WiFi access point configured with SSID: ESP32-DualMode, channel: 6, auth mode: WPA2_PSK
I (396) SpeedWifi: WiFi started
I (403) SpeedWifi: Connecting to WiFi network...

Dual Mode WiFi started!

Station (client) mode:
  Connecting to: MyNetwork
  Status: Connecting...

Access Point mode:
  SSID: ESP32-DualMode
  Password: password123
  IP Address: 192.168.4.1

I (2312) SpeedWifi: WiFi connected to AP
Station connected to network!

I (3521) SpeedWifi: Got IP address: 192.168.1.105
Got IP address: 192.168.1.105
Station connection complete!

I (12539) SpeedWifi: Station 11:22:33:44:55:66 joined, AID=1
Station connected to our access point!
  MAC: 11:22:33:44:55:66
  AID: 1
```

## Network Topology

When operating in dual mode:

1. The ESP32 connects to your existing WiFi network like any other client
2. The ESP32 creates its own WiFi network (access point)
3. Devices connecting to the ESP32's access point can:
   - Communicate with the ESP32
   - Potentially access the wider network through the ESP32 (if you implement routing)

## Troubleshooting

If you have trouble with dual mode:

1. Try using different channels for your existing network and the ESP32's access point
2. Ensure your ESP32 has sufficient power (dual mode requires more power)
3. Check for potential IP range conflicts between networks
4. Verify your network credentials are correct

## Further Development

Try modifying the example to:

1. Implement a web server accessible from both networks
2. Create a network bridge to allow devices connected to the AP to access the internet
3. Set up captive portal for easier WiFi configuration
4. Add more sophisticated network management features
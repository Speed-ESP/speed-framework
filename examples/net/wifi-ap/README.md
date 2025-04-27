# WiFi Access Point Example

This example demonstrates how to use the Speed Framework to configure an ESP32 device as a WiFi access point.

## Features

- Sets up the ESP32 as a WiFi access point
- Configures security settings (WPA2-PSK by default)
- Handles station connection/disconnection events
- Shows how to use the SmartEnum classes for WiFi configuration

## Hardware Required

- ESP32 development board
- USB cable for programming and power
- WiFi-capable device (smartphone, laptop, etc.) to test the connection

## Setup

1. Update the access point configuration in `main.cpp` if desired
2. Build and flash the example to your ESP32
3. Connect to the created access point from another device

## Code Overview

The example demonstrates:

- Setting up the WiFi service in access point mode
- Configuring access point parameters (SSID, password, channel, etc.)
- Handling station connection/disconnection events

## How It Works

The example creates a custom event handler to respond to WiFi events such as station connection and disconnection. It then configures the WiFi interface in access point mode with the specified parameters.

## Expected Output

If everything works correctly, you should see output similar to the following on the serial monitor:

```
I (345) SpeedNet: SpeedNet initialized
I (352) SpeedWifi: Initializing SpeedWifi
I (362) SpeedWifi: WiFi event handlers registered
I (367) SpeedWifi: SpeedWifi initialized successfully
I (374) SpeedWifi: WiFi mode set to AP
I (381) SpeedWifi: WiFi access point configured with SSID: ESP32-AP, channel: 6, auth mode: WPA2_PSK
I (389) SpeedWifi: WiFi started
Access point started!
  SSID: ESP32-AP
  Password: password123
  IP Address: 192.168.4.1

Waiting for stations to connect...

I (12539) SpeedWifi: Station 11:22:33:44:55:66 joined, AID=1
Station connected!
  MAC: 11:22:33:44:55:66
  AID: 1
```

When a device connects to your access point, you'll see a message showing the MAC address of the connected device.

## Troubleshooting

If you have trouble with the access point:

1. Ensure there's no channel conflict with other nearby access points
2. Try changing the channel in the configuration
3. Verify that your client device supports the configured security mode
4. Check the console output for specific error messages

## Further Development

Try modifying the example to:

1. Implement password protection or change security settings
2. Add a captive portal for WiFi setup
3. Create a web server on the access point
4. Set up DHCP server with custom IP range
# WiFi Client Example

This example demonstrates how to use the Speed Framework to connect an ESP32 device to a WiFi network as a client (station mode).

## Features

- Connects to a specified WiFi network
- Handles connection events
- Displays connection information
- Demonstrates error handling
- Shows how to use the SmartEnum classes for WiFi configuration

## Hardware Required

- ESP32 development board
- USB cable for programming and power

## Setup

1. Update the WiFi network credentials in `main.cpp` with your network's SSID and password
2. Build and flash the example to your ESP32

## Code Overview

The example demonstrates:

- Setting up the WiFi service in station mode
- Configuring the connection parameters
- Connecting to the WiFi network
- Handling WiFi events (connection, disconnection, IP acquisition)
- Displaying connection information

## How It Works

The example creates a custom event handler to respond to WiFi events such as connection success/failure and IP address acquisition. It then configures the WiFi interface in station mode and attempts to connect to the specified network.

## Expected Output

If everything works correctly, you should see output similar to the following on the serial monitor:

```
I (345) SpeedNet: SpeedNet initialized
I (352) SpeedWifi: Initializing SpeedWifi
I (362) SpeedWifi: WiFi event handlers registered
I (367) SpeedWifi: SpeedWifi initialized successfully
I (374) SpeedWifi: WiFi mode set to STA
I (381) SpeedWifi: WiFi station configured with SSID: MyNetwork
I (389) SpeedWifi: WiFi started
I (396) SpeedWifi: Connecting to WiFi network...
I (1423) SpeedWifi: WiFi connected to AP
Connected to WiFi network!
I (2512) SpeedWifi: Got IP address: 192.168.1.105
Got IP address: 192.168.1.105
Connected successfully!
Connection information:
  SSID: MyNetwork
  Channel: 6
  RSSI: -67
  Authentication Mode: WPA2_PSK
```

## Troubleshooting

If you have trouble connecting:

1. Verify that your WiFi credentials are correct
2. Check that your ESP32 is within range of the WiFi network
3. Ensure your WiFi network is operational
4. Check the console output for specific error messages

## Further Development

Try modifying the example to:

1. Reconnect automatically if disconnected
2. Connect to the strongest available network from a list
3. Implement a more sophisticated error handling mechanism
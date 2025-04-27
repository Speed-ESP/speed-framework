# WiFi Scanner Example

This example demonstrates how to use the Speed Framework to scan for available WiFi networks with an ESP32 device.

## Features

- Scans for available WiFi networks
- Displays detailed information about discovered networks
- Shows how to customize scan parameters
- Demonstrates event-based handling of scan results
- Uses SmartEnum classes for type-safe WiFi configuration

## Hardware Required

- ESP32 development board
- USB cable for programming and power

## Setup

1. Build and flash the example to your ESP32
2. Open the serial monitor to view scan results

## Code Overview

The example demonstrates:

- Setting up the WiFi service in station mode for scanning
- Configuring scan parameters (active/passive, show hidden networks)
- Executing the scan operation
- Handling scan completion events
- Processing and displaying scan results 

## How It Works

The example creates a custom event handler to receive scan completion events. When a scan is completed, it processes the results and displays information about each discovered network including SSID, signal strength (RSSI), channel, and security settings.

## Expected Output

If everything works correctly, you should see output similar to the following on the serial monitor:

```
I (345) SpeedNet: SpeedNet initialized
I (352) SpeedWifi: Initializing SpeedWifi
I (362) SpeedWifi: WiFi event handlers registered
I (367) SpeedWifi: SpeedWifi initialized successfully
I (374) SpeedWifi: WiFi mode set to STA
I (381) SpeedWifi: WiFi started
I (386) SpeedWifi: WiFi scan started with scan type: ACTIVE
Starting WiFi scan...

I (2481) SpeedWifi: Scan done: found 8 access points
Scan complete! Found 8 networks:

Network 1:
  SSID: MyHomeNetwork
  RSSI: -42 dBm (Excellent)
  Channel: 6
  Auth Mode: WPA2_PSK
  Hidden: No

Network 2:
  SSID: Neighbor_WiFi
  RSSI: -67 dBm (Good)
  Channel: 11
  Auth Mode: WPA_WPA2_PSK
  Hidden: No

...

Scan complete. To scan again, reset the device.
```

## Interpreting RSSI Values

The example includes a function to interpret RSSI (signal strength) values:

- **Excellent**: -50 dBm or better
- **Good**: -50 to -60 dBm
- **Fair**: -60 to -70 dBm
- **Weak**: -70 to -80 dBm
- **Poor**: -80 dBm or worse

## Troubleshooting

If you don't see any networks:

1. Ensure there are WiFi networks in range
2. Try changing the scan type to passive or increasing the scan time
3. Check if your ESP32 antenna is properly connected (if using an external antenna)
4. Verify that the serial monitor is working properly

## Further Development

Try modifying the example to:

1. Implement automatic reconnection to the strongest network
2. Filter networks by security type or signal strength
3. Create a more sophisticated network selection algorithm
4. Store preferred networks in non-volatile storage
#include "net/modbus/modbus_device_manager.hpp"
#include "net/modbus/transport/modbus_uart_transport.hpp"
#include "net/modbus/transport/modbus_tcp_transport.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <vector>
#include <memory>

using namespace speed::net::modbus;
static const char* TAG = "ModbusExample";

// Example callback for value updates
void handleValueUpdate(const ModbusValue& value) {
    ESP_LOGI(TAG, "Register 0x%04X updated: value = %d (0x%04X)", 
             value.address, value.value, value.value);
}

// Example callback for device discovery
void handleDeviceDiscovery(uint8_t address, bool present) {
    if (present) {
        ESP_LOGI(TAG, "Found Modbus device at address %d", address);
    }
}

extern "C" void app_main(void) {
    // Create transport layer (UART in this example)
    auto transport = std::make_shared<ModbusUartTransport>(
        UART_NUM_1,    // UART port
        GPIO_NUM_17,   // TX pin
        GPIO_NUM_16,   // RX pin
        UART_PIN_NO_CHANGE, // RTS pin
        9600          // Baud rate
    );

    // Create and configure Modbus master
    ModbusConfig config;
    config.responseTimeoutMs = 2000;
    auto master = std::make_shared<ModbusMaster>(transport, config);
    
    if (!master->begin()) {
        ESP_LOGE(TAG, "Failed to initialize ModbusMaster");
        return;
    }

    // Create device manager
    ModbusDeviceManager deviceManager(master);

    // Scan for devices
    deviceManager.scanNetwork(1, 10, handleDeviceDiscovery);
    vTaskDelay(pdMS_TO_TICKS(3000));  // Wait for scan to complete

    // Add known devices
    auto device1 = deviceManager.addDevice(1, "MPPT Controller");
    auto device2 = deviceManager.addDevice(2, "Battery Monitor");

    if (device1) {
        // Configure polling for important registers
        device1->enablePolling(0x0100, 1000, handleValueUpdate);  // Poll every second
        device1->enablePolling(0x0101, 5000, handleValueUpdate);  // Poll every 5 seconds

        // Read multiple registers
        device1->readMultipleHoldingRegisters(0x0100, 10, 
            [](const std::vector<ModbusValue>& values) {
                ESP_LOGI(TAG, "Read %d registers:", values.size());
                for (const auto& value : values) {
                    ESP_LOGI(TAG, "  Register 0x%04X = %d", value.address, value.value);
                }
            });
    }

    if (device2) {
        // Example of writing registers
        device2->writeHoldingRegister(0x0200, 42, handleValueUpdate);
    }

    // Main loop
    while (1) {
        // Print network statistics every 30 seconds
        static uint32_t lastStats = 0;
        uint32_t now = xTaskGetTickCount();
        
        if ((now - lastStats) >= pdMS_TO_TICKS(30000)) {
            const auto& stats = deviceManager.getNetworkStatistics();
            ESP_LOGI(TAG, "Network Statistics:");
            ESP_LOGI(TAG, "  Successful transactions: %d", stats.successfulTransactions);
            ESP_LOGI(TAG, "  Failed transactions: %d", stats.failedTransactions);
            ESP_LOGI(TAG, "  Retries: %d", stats.retries);
            ESP_LOGI(TAG, "  Timeouts: %d", stats.timeouts);
            lastStats = now;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
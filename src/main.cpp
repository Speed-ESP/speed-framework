#include <esp_log.h>
#include <functional>
#include "net/modbus/modbus_defs.hpp"
#include "net/modbus/modbus_config.hpp"
#include "net/modbus/modbus_master.hpp"
#include "net/modbus/modbus_device_manager.hpp"
#include "net/modbus/transport/modbus_uart_transport.hpp"
#include "net/modbus/transport/modbus_tcp_transport.hpp"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "net/speed_net.hpp";

using namespace speed::net::modbus;
using namespace speed::net;
const char *TAG = "Main";
extern "C"
{
    void app_main(void);
}

std::function<void(void)> hellow = []()
{
    ESP_LOGI("Main", "Hello, ESP32!");
};

void app_main()
{

    auto wifi = SpeedWifi::setup();

    wifi.setMode(SpeedWifiMode::STA)
        .configureStation("Red#1", "isa0124.")
        .start();

    wifi.connect();

    while(!wifi.waitForConnection(10000)){
        ESP_LOGI(TAG, "Failed to connect to WiFi");
        vTaskDelay(pdMS_TO_TICKS(2000));
        wifi.connect();
    }

    auto info = wifi.getConnectionInfo();
    ESP_LOGI(TAG,"Connected to: %s, Channel: %d, RSSI: %d",  info.ssid, info.primary, info.rssi);

    // Create TCP transport
    auto transport = std::make_shared<ModbusTcpTransport>("192.168.100.76", 502, true);
    transport->setKeepAlive(true);
    // const auto rs485config = std::make_shared<Rs485UartConfig>();
    // // Alternative: Create RTU transport
    // auto transport = std::make_shared<ModbusUartTransport>(
    //     UART_NUM_0,
    //     2,   // TX
    //     3,   // RX
    //     rs485config
    // );

    // Configure master
    ModbusConfig config;
    config.responseTimeoutMs = 2000;
    config.queueSize = 100;
    config.taskPriority = 7;

    auto master = std::make_shared<ModbusMaster>(transport, config);

    while (!master->begin())
    {
        ESP_LOGE(TAG, "Failed to start Modbus master");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    // Create device manager
    auto deviceManager = std::make_shared<ModbusDeviceManager>(master);
    
    // Configure the default polling interval (applies to registers with 0 interval)
    deviceManager->setPollInterval(1000); // 1 second default polling interval
    
    // Start the centralized polling task - manages polling for all devices
    if (!deviceManager->startPolling()) {
        ESP_LOGE(TAG, "Failed to start polling task");
    }

    // Add devices
    auto slave1 = deviceManager->addDevice(1, "Slave 1");
    // auto slave2 = deviceManager->addDevice(2, "Slave 2");

    // Configure polling for each device - these will be managed by the device manager
    slave1->enablePolling(1, 2000, [](const ModbusValue &value) {
        if (value.valid) {
            ESP_LOGI(TAG, "Slave 1 - Register 0: %d", value.value);
        }
    });
    
    // slave1->enablePolling(1, 1000, [](const ModbusValue &value) {
    //     if (value.valid) {
    //         ESP_LOGI(TAG, "Slave 1 - Register 1: %d", value.value);
    //     }
    // });
    
    // slave2->enablePolling(0, 2000, [](const ModbusValue &value) {
    //     if (value.valid) {
    //         ESP_LOGI(TAG, "Slave 2 - Register 0: %d", value.value);
    //     }
    // });

    // Main loop
    while (1) {
        // Print network statistics every 30 seconds
        static uint32_t lastStats = 0;
        uint32_t now = xTaskGetTickCount();
        
        if ((now - lastStats) >= pdMS_TO_TICKS(30000)) {
            const auto& stats = deviceManager->getNetworkStatistics();
            ESP_LOGI(TAG, "Network Statistics:");
            ESP_LOGI(TAG, "  Successful transactions: %li", stats.successfulTransactions);
            ESP_LOGI(TAG, "  Failed transactions: %li", stats.failedTransactions);
            ESP_LOGI(TAG, "  Retries: %li", stats.retries);
            ESP_LOGI(TAG, "  Timeouts: %li", stats.timeouts);
            lastStats = now;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
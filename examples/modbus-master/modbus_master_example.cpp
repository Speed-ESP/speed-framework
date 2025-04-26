#include <memory>
#include <string>
#include "esp_log.h"
#include "net/modbus/modbus_master.hpp"
#include "net/modbus/modbus_device_manager.hpp"
#include "net/modbus/transport/modbus_tcp_transport.hpp"
#include "net/modbus/transport/modbus_uart_transport.hpp"

using namespace speed::net::modbus;

static const char* TAG = "ModbusMaster";

// Device-specific register definitions
struct DeviceRegisters {
    static constexpr uint16_t INPUT_VOLTAGE = 0x0100;
    static constexpr uint16_t OUTPUT_VOLTAGE = 0x0101;
    static constexpr uint16_t BATTERY_LEVEL = 0x0102;
    static constexpr uint16_t TEMPERATURE = 0x0103;
    static constexpr uint16_t CONTROL_MODE = 0x0200;
    static constexpr uint16_t OUTPUT_ENABLE = 0x0201;
};

class ModbusMasterExample {
public:
    ModbusMasterExample() {
        initializeMaster();
        setupDevices();
        configurePolling();
    }

    void run() {
        // Write control values
        writeControlValues();

        // Read values on demand
        readValues();

        // Keep the application running
        while (true) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

private:
    void initializeMaster() {
        // Create TCP transport
        auto tcpTransport = std::make_shared<ModbusTcpTransport>("192.168.1.100", 502);
        
        // Alternative: Create RTU transport
        /*
        auto rtuTransport = std::make_shared<ModbusUartTransport>(
            UART_NUM_1,
            GPIO_NUM_17,    // TX
            GPIO_NUM_16,    // RX
            GPIO_NUM_4,     // RTS
            9600           // Baud rate
        );
        */

        // Configure master
        ModbusConfig config;
        config.responseTimeoutMs = 2000;
        config.queueSize = 32;
        config.taskPriority = 5;
        
        _master = std::make_shared<ModbusMaster>(tcpTransport, config);
        if (!_master->begin()) {
            ESP_LOGE(TAG, "Failed to start Modbus master");
            return;
        }

        // Create device manager
        _deviceManager = std::make_shared<ModbusDeviceManager>(_master);
    }

    void setupDevices() {
        // Add multiple devices with different addresses
        _device1 = _deviceManager->addDevice(1, "Power Controller");
        _device2 = _deviceManager->addDevice(2, "Battery Monitor");
        _device3 = _deviceManager->addDevice(3, "Temperature Sensor");

        // Configure device-specific settings
        _device1->setMaxCacheAge(1000);  // 1 second cache
        _device2->setMaxCacheAge(2000);  // 2 second cache
        _device3->setMaxCacheAge(5000);  // 5 second cache

        // Set up value scaling
        _device1->setValueScaling(DeviceRegisters::INPUT_VOLTAGE, 0.1f);
        _device1->setValueScaling(DeviceRegisters::OUTPUT_VOLTAGE, 0.1f);
        _device2->setValueScaling(DeviceRegisters::BATTERY_LEVEL, 0.1f);
    }

    void configurePolling() {
        // Regular polling for device 1
        _device1->enablePolling(DeviceRegisters::INPUT_VOLTAGE, 1000,
            [](const ModbusValue& value) {
                if (value.valid) {
                    ESP_LOGI(TAG, "Input Voltage: %.1fV", value.value * 0.1f);
                }
            });

        _device1->enablePolling(DeviceRegisters::OUTPUT_VOLTAGE, 1000,
            [](const ModbusValue& value) {
                if (value.valid) {
                    ESP_LOGI(TAG, "Output Voltage: %.1fV", value.value * 0.1f);
                }
            });

        // Battery level monitoring
        _device2->enablePolling(DeviceRegisters::BATTERY_LEVEL, 5000,
            [](const ModbusValue& value) {
                if (value.valid) {
                    ESP_LOGI(TAG, "Battery Level: %.1f%%", value.value * 0.1f);
                }
            });

        // Temperature monitoring with threshold
        _device3->enablePolling(DeviceRegisters::TEMPERATURE, 2000,
            [](const ModbusValue& value) {
                if (value.valid) {
                    float temp = value.value / 10.0f;
                    ESP_LOGI(TAG, "Temperature: %.1f°C", temp);
                    if (temp > 40.0f) {
                        ESP_LOGW(TAG, "High temperature warning!");
                    }
                }
            });

        // Set up value change monitoring
        _device1->onValueChange(DeviceRegisters::CONTROL_MODE,
            [](const ModbusValue& oldValue, const ModbusValue& newValue) {
                ESP_LOGI(TAG, "Control mode changed: %d -> %d",
                         oldValue.value, newValue.value);
            });
    }

    void writeControlValues() {
        // Write single register with callback
        _device1->writeSingleRegister(DeviceRegisters::CONTROL_MODE, 1,
            [](const ModbusValue& value) {
                if (value.valid) {
                    ESP_LOGI(TAG, "Control mode set successfully");
                }
            });

        // Write multiple registers
        std::vector<uint16_t> values = {1, 100, 500};
        _device1->writeMultipleRegisters(DeviceRegisters::OUTPUT_ENABLE, values,
            [](bool success) {
                if (success) {
                    ESP_LOGI(TAG, "Multiple registers written successfully");
                }
            });
    }

    void readValues() {
        // Read single register
        _device1->readHoldingRegister(DeviceRegisters::CONTROL_MODE,
            [](const ModbusValue& value) {
                if (value.valid) {
                    ESP_LOGI(TAG, "Control Mode: %d", value.value);
                }
            });

        // Read multiple registers
        _device2->readMultipleHoldingRegisters(DeviceRegisters::BATTERY_LEVEL, 2,
            [](const std::vector<ModbusValue>& values) {
                if (!values.empty() && values[0].valid) {
                    ESP_LOGI(TAG, "Battery Level: %.1f%%", values[0].value * 0.1f);
                }
            });

        // Read using cache
        _device3->readHoldingRegisterCached(DeviceRegisters::TEMPERATURE,
            [](const ModbusValue& value) {
                if (value.valid) {
                    ESP_LOGI(TAG, "Cached Temperature: %.1f°C",
                             value.value / 10.0f);
                }
            });
    }

private:
    std::shared_ptr<ModbusMaster> _master;
    std::shared_ptr<ModbusDeviceManager> _deviceManager;
    std::shared_ptr<ModbusDevice> _device1;
    std::shared_ptr<ModbusDevice> _device2;
    std::shared_ptr<ModbusDevice> _device3;
};

extern "C" void app_main() {
    ModbusMasterExample example;
    example.run();
}
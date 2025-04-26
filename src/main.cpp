#include <esp_log.h>
#include <functional>
#include "net/modbus/modbus_defs.hpp"
#include "net/modbus/modbus_config.hpp"
#include "net/modbus/modbus_master.hpp"
#include "net/modbus/modbus_device_manager.hpp"
#include "net/modbus/transport/modbus_uart_transport.hpp"
#include "net/modbus/transport/modbus_tcp_transport.hpp"
#include "driver/gpio.h"


using namespace speed::net::modbus;

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

    // Create TCP transport
    // auto tcpTransport = std::make_shared<ModbusTcpTransport>("192.168.1.100", 502);

    // Alternative: Create RTU transport
    auto rtuTransport = std::make_shared<ModbusUartTransport>(
        UART_NUM_0,
        2,   // TX
        3,   // RX
        4,   // RTS
        9600 // Baud rate
    );

    // Configure master
    ModbusConfig config;
    config.responseTimeoutMs = 2000;
    config.queueSize = 32;
    config.taskPriority = 5;

    auto master = std::make_shared<ModbusMaster>(rtuTransport, config);
    if (!master->begin())
    {
        ESP_LOGE(TAG, "Failed to start Modbus master");
        return;
    }

    // Create device manager
    auto deviceManager = std::make_shared<ModbusDeviceManager>(master);
    hellow();
}

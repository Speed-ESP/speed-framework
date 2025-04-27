#include <memory>
#include <string>
#include "esp_log.h"
#include "net/modbus/modbus_master.hpp"
#include "net/modbus/modbus_device_manager.hpp"
#include "net/modbus/transport/modbus_tcp_transport.hpp"
#include "net/modbus/transport/modbus_uart_transport.hpp"
#include "net/modbus/packager/modbus_packager_factory.hpp"

using namespace speed::net::modbus;

static const char* TAG = "ModbusPackagerExample";

/**
 * This example demonstrates how to use different packagers with ModbusMaster
 * to support different Modbus protocols (RTU, ASCII, MBAP) over different
 * transport layers (TCP, UART).
 */
void modbus_packager_example() {
    // Example 1: Using default packager (simple approach)
    // ------------------------------------------------
    // Each transport provides its appropriate default packager
    
    // TCP transport automatically provides MBAP packager
    auto defaultTcpTransport = std::make_shared<ModbusTcpTransport>("192.168.1.100", 502);
    auto defaultTcpMaster = std::make_shared<ModbusMaster>(defaultTcpTransport);
    
    // UART transport automatically provides RTU packager
    auto defaultUartTransport = std::make_shared<ModbusUartTransport>(
        UART_NUM_1, 
        GPIO_NUM_17,   // TX pin
        GPIO_NUM_16,   // RX pin
        std::make_shared<RtuUartConfig>()
    );
    auto defaultUartMaster = std::make_shared<ModbusMaster>(defaultUartTransport);
    
    ESP_LOGI(TAG, "Default packagers demonstrate the simplest approach");
    
    // Example 2: TCP transport with custom packager (ASCII instead of MBAP)
    // ---------------------------------------------------------------
    auto tcpTransport = std::make_shared<ModbusTcpTransport>("192.168.1.101", 502);
    
    // Create ASCII packager (non-standard for TCP)
    auto asciiPackager = ModbusPackagerFactory::createAsciiPackager();
    
    // Set up configuration with custom packager
    ModbusConfig tcpConfig;
    tcpConfig.packager = asciiPackager;
    
    // Create master with custom packager
    auto tcpMaster = std::make_shared<ModbusMaster>(tcpTransport, tcpConfig);
    if (!tcpMaster->begin()) {
        ESP_LOGE(TAG, "Failed to initialize TCP master with ASCII packager");
        return;
    }
    
    // Example 3: UART transport with custom packager (MBAP instead of RTU)
    // ---------------------------------------------------------------
    auto uartTransport = std::make_shared<ModbusUartTransport>(
        UART_NUM_2, 
        GPIO_NUM_18,   // TX pin
        GPIO_NUM_19,   // RX pin
        std::make_shared<UartConfig>()
    );
    
    // Create MBAP packager (non-standard for UART)
    auto mbapPackager = ModbusPackagerFactory::createMbapPackager();
    
    // Set up configuration with packager
    ModbusConfig mbapConfig;
    mbapConfig.packager = mbapPackager;
    
    // Create master with custom packager
    auto mbapMaster = std::make_shared<ModbusMaster>(uartTransport, mbapConfig);
    if (!mbapMaster->begin()) {
        ESP_LOGE(TAG, "Failed to initialize UART master with MBAP packager");
        return;
    }
    
    // Example 4: Changing packager at runtime
    // ------------------------------------
    ESP_LOGI(TAG, "Changing packager at runtime");
    tcpMaster->setPackager(ModbusPackagerFactory::createRtuPackager());
    
    // For communication, use these masters as normal
    ESP_LOGI(TAG, "Modbus packager example complete");
}
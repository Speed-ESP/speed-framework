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
    // Example 1: TCP transport with MBAP packager (standard combination)
    // ---------------------------------------------------------------
    // Create TCP transport
    auto tcpTransport = std::make_shared<ModbusTcpTransport>("192.168.1.100", 502);
    
    // Create MBAP packager (standard for TCP)
    auto mbapPackager = ModbusPackagerFactory::createMbapPackager();
    
    // Set up configuration with packager
    ModbusConfig tcpConfig;
    tcpConfig.packager = mbapPackager;
    
    // Create master with packager
    auto tcpMaster = std::make_shared<ModbusMaster>(tcpTransport, tcpConfig);
    if (!tcpMaster->begin()) {
        ESP_LOGE(TAG, "Failed to initialize TCP master");
        return;
    }
    
    // Example 2: UART transport with RTU packager (standard combination)
    // ---------------------------------------------------------------
    // Create UART transport
    auto uartTransport = std::make_shared<ModbusUartTransport>(
        UART_NUM_1, 
        GPIO_NUM_17,   // TX pin
        GPIO_NUM_16,   // RX pin
        std::make_shared<RtuUartConfig>()
    );
    
    // Create RTU packager (standard for UART)
    auto rtuPackager = ModbusPackagerFactory::createRtuPackager();
    
    // Set up configuration with packager
    ModbusConfig rtuConfig;
    rtuConfig.packager = rtuPackager;
    
    // Create master with packager
    auto rtuMaster = std::make_shared<ModbusMaster>(uartTransport, rtuConfig);
    if (!rtuMaster->begin()) {
        ESP_LOGE(TAG, "Failed to initialize RTU master");
        return;
    }
    
    // Example 3: UART transport with ASCII packager (non-standard but valid combination)
    // -----------------------------------------------------------------------------
    // Create UART transport (same as above)
    auto asciiUartTransport = std::make_shared<ModbusUartTransport>(
        UART_NUM_2, 
        GPIO_NUM_18,   // TX pin
        GPIO_NUM_19,   // RX pin
        std::make_shared<UartConfig>()
    );
    
    // Create ASCII packager - less common but supported
    auto asciiPackager = ModbusPackagerFactory::createAsciiPackager();
    
    // Set up configuration with packager
    ModbusConfig asciiConfig;
    asciiConfig.packager = asciiPackager;
    
    // Create master with packager
    auto asciiMaster = std::make_shared<ModbusMaster>(asciiUartTransport, asciiConfig);
    if (!asciiMaster->begin()) {
        ESP_LOGE(TAG, "Failed to initialize ASCII master");
        return;
    }
    
    // Example 4: TCP transport with RTU packager (unusual combination)
    // -------------------------------------------------------------
    // This demonstrates the flexibility of separating transport from packager
    auto tcpTransport2 = std::make_shared<ModbusTcpTransport>("192.168.1.101", 502);
    
    // Use RTU packager with TCP transport
    ModbusConfig tcpRtuConfig;
    tcpRtuConfig.packager = ModbusPackagerFactory::createRtuPackager();
    
    // Create master with unusual combination
    auto tcpRtuMaster = std::make_shared<ModbusMaster>(tcpTransport2, tcpRtuConfig);
    if (!tcpRtuMaster->begin()) {
        ESP_LOGE(TAG, "Failed to initialize TCP+RTU master");
        return;
    }
    
    // Example 5: Changing packager at runtime
    // ------------------------------------
    // You can also change the packager at runtime if needed
    ESP_LOGI(TAG, "Changing packager from RTU to ASCII");
    tcpRtuMaster->setPackager(ModbusPackagerFactory::createAsciiPackager());
    
    // Example 6: Using default packagers (simplified approach)
    // ---------------------------------------------------
    // Let the system choose default packagers based on transport type
    auto defaultTcpTransport = std::make_shared<ModbusTcpTransport>("192.168.1.102", 502);
    auto defaultUartTransport = std::make_shared<ModbusUartTransport>(
        UART_NUM_1, 
        GPIO_NUM_21,   // TX pin
        GPIO_NUM_22,   // RX pin
        std::make_shared<UartConfig>()
    );
    
    // Create masters without specifying packagers
    auto defaultTcpMaster = std::make_shared<ModbusMaster>(defaultTcpTransport);  // Will use MBAP by default
    auto defaultUartMaster = std::make_shared<ModbusMaster>(defaultUartTransport); // Will use RTU by default
    
    // For communication, use these masters as normal
    ESP_LOGI(TAG, "Modbus packager example complete");
}
#include <iostream>
#include <memory>
#include <net/modbus/modbus_master.hpp>
#include <net/modbus/transport/modbus_tcp_transport.hpp>
#include <net/modbus/transport/modbus_uart_transport.hpp>
#include <net/modbus/transport/modbus_packager_factory.hpp>

using namespace speed::net::modbus;

void example_using_custom_packager()
{
    // Example 1: TCP transport with ASCII packager (non-standard combination)
    // -----------------------------------------------------------------------
    // Create TCP transport
    auto tcpTransport = std::make_shared<ModbusTcpTransport>("192.168.1.100", 502);
    
    // Create configuration with custom packager (ASCII for TCP, which is unusual)
    ModbusConfig tcpConfig;
    tcpConfig.packager = ModbusPackagerFactory::createAsciiPackager();
    
    // Create master with custom packager
    ModbusMaster tcpMaster(tcpTransport, tcpConfig);
    
    // Example 2: UART transport with MBAP packager (non-standard combination)
    // -----------------------------------------------------------------------
    // Create UART transport
    auto uartTransport = std::make_shared<ModbusUartTransport>(
        UART_NUM_1, 
        GPIO_NUM_17, 
        GPIO_NUM_16, 
        GPIO_NUM_18, 
        9600
    );
    
    // Create configuration with custom packager (MBAP for UART, which is unusual)
    ModbusConfig uartConfig;
    uartConfig.packager = ModbusPackagerFactory::createMbapPackager();
    
    // Create master with custom packager
    ModbusMaster uartMaster(uartTransport, uartConfig);
    
    // Example 3: Using default packagers (more typical)
    // ------------------------------------------------
    // For TCP, the default is MBAP
    auto defaultTcpTransport = std::make_shared<ModbusTcpTransport>("192.168.1.101", 502);
    ModbusMaster defaultTcpMaster(defaultTcpTransport); // Uses MBAP by default
    
    // For UART, the default is RTU
    auto defaultUartTransport = std::make_shared<ModbusUartTransport>(
        UART_NUM_2, 
        GPIO_NUM_19, 
        GPIO_NUM_20
    );
    ModbusMaster defaultUartMaster(defaultUartTransport); // Uses RTU by default
}
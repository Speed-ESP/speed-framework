# Modbus Packager System

The Speed Framework Modbus library now supports a flexible packaging system that separates the protocol formatting (packager) from the transport layer. This allows for greater flexibility in how Modbus messages are formatted and parsed, independent of the transport medium used.

## Packager Types

The library supports three packager types:

1. **MBAP (Modbus TCP) Packager** - Standard for Modbus TCP communications, includes transaction ID, protocol ID, and length field.
2. **RTU Packager** - Binary format with CRC, typically used with serial communications.
3. **ASCII Packager** - Text-based format with LRC, for systems that only support ASCII communications.

## Benefits of Separate Packagers

- **Flexibility**: Use any packager with any transport type
- **Extensibility**: Easily add new packager types without modifying transport code
- **Clarity**: Clear separation of concerns between transport and protocol formatting
- **Runtime Switching**: Change packager types during runtime without recreating connections

## Using Packagers

### Default Behavior

By default, the system will automatically select the appropriate packager based on the transport type:

- TCP transports will use MBAP packager
- UART transports will use RTU packager

```cpp
// Let the system choose the default packager
auto tcpTransport = std::make_shared<ModbusTcpTransport>("192.168.1.100", 502);
auto tcpMaster = std::make_shared<ModbusMaster>(tcpTransport);  // Uses MBAP by default

auto uartTransport = std::make_shared<ModbusUartTransport>(UART_NUM_1, GPIO_NUM_17, GPIO_NUM_16);
auto rtuMaster = std::make_shared<ModbusMaster>(uartTransport); // Uses RTU by default
```

### Explicit Packager Selection

You can explicitly specify a packager in the ModbusConfig:

```cpp
// Create a packager
auto asciiPackager = ModbusPackagerFactory::createAsciiPackager();

// Configure and create master with packager
ModbusConfig config;
config.packager = asciiPackager;
auto master = std::make_shared<ModbusMaster>(transport, config);
```

### Runtime Packager Switching

You can change the packager at runtime:

```cpp
// Switch from default packager to ASCII packager
master->setPackager(ModbusPackagerFactory::createAsciiPackager());

// Switch to RTU packager
master->setPackager(ModbusPackagerFactory::createRtuPackager());

// Switch to MBAP packager
master->setPackager(ModbusPackagerFactory::createMbapPackager());
```

## Advanced Use Cases

### Non-Standard Combinations

The separation of packagers from transports allows for non-standard combinations that might be required in special situations:

- **TCP with RTU**: Some devices expect RTU-formatted messages over TCP
- **TCP with ASCII**: For devices that only accept ASCII over TCP
- **UART with MBAP**: When a serial device requires MBAP formatting

### Custom Packagers

You can create your own packager by implementing the `ModbusPackager` interface. This is useful for proprietary Modbus extensions or modified protocols.

## Example

See the full example in `examples/modbus-master/modbus_packager_example.cpp` for practical demonstrations of packager usage.
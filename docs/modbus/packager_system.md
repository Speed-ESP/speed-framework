# Modbus Packager System

The Speed Framework Modbus library supports a flexible packaging system that separates the protocol formatting (packager) from the transport layer. This allows for greater flexibility in how Modbus messages are formatted and parsed, independent of the transport medium used.

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

## Package Format Tables

### 1. MBAP (Modbus TCP) Packager

The MBAP packager adds a 7-byte header to each Modbus PDU.

**MBAP Frame Format:**

| Field | Length | Description |
|-------|--------|-------------|
| Transaction ID | 2 bytes | For transaction pairing, client increments for each request |
| Protocol ID | 2 bytes | Always 0 for Modbus TCP |
| Length | 2 bytes | Number of bytes following (unit ID + PDU length) |
| Unit ID | 1 byte | Slave address (typically 1-247, 0 for broadcast) |
| PDU | N bytes | Function code (1 byte) + Data (varies) |

**Example: Read Holding Registers (Function 03)**

Request to read 2 registers from address 0x0100 of device 1:

```
Transaction ID: 0x0001
Protocol ID:    0x0000
Length:         0x0006
Unit ID:        0x01
Function Code:  0x03
Start Address:  0x0100
Quantity:       0x0002
```

Hexadecimal: `00 01 00 00 00 06 01 03 01 00 00 02`

Response with values 0x0064 and 0x00C8:

```
Transaction ID: 0x0001
Protocol ID:    0x0000
Length:         0x0007
Unit ID:        0x01
Function Code:  0x03
Byte Count:     0x04
Data:           0x00 0x64 0x00 0xC8
```

Hexadecimal: `00 01 00 00 00 07 01 03 04 00 64 00 C8`

**Example: Write Multiple Registers (Function 16)**

Request to write 2 registers at address 0x0200 of device 1 with values 0x0064 and 0x00C8:

```
Transaction ID: 0x0002
Protocol ID:    0x0000
Length:         0x000B
Unit ID:        0x01
Function Code:  0x10
Start Address:  0x0200
Quantity:       0x0002
Byte Count:     0x04
Data:           0x00 0x64 0x00 0xC8
```

Hexadecimal: `00 02 00 00 00 0B 01 10 02 00 00 02 04 00 64 00 C8`

Response confirming write:

```
Transaction ID: 0x0002
Protocol ID:    0x0000
Length:         0x0006
Unit ID:        0x01
Function Code:  0x10
Start Address:  0x0200
Quantity:       0x0002
```

Hexadecimal: `00 02 00 00 00 06 01 10 02 00 00 02`

### 2. RTU Packager

RTU frames use binary format with a CRC checksum at the end.

**RTU Frame Format:**

| Field | Length | Description |
|-------|--------|-------------|
| Slave Address | 1 byte | Device address (1-247, 0 for broadcast) |
| PDU | N bytes | Function code (1 byte) + Data (varies) |
| CRC | 2 bytes | CRC-16 checksum (low byte first) |

**Example: Read Input Registers (Function 04)**

Request to read 3 input registers from address 0x0800 of device 2:

```
Slave Address: 0x02
Function Code: 0x04
Start Address: 0x0800
Quantity:      0x0003
CRC:           0x3B 0x91
```

Hexadecimal: `02 04 08 00 00 03 3B 91`

Response with values 0x0100, 0x0200, and 0x0300:

```
Slave Address: 0x02
Function Code: 0x04
Byte Count:    0x06
Data:          0x01 0x00 0x02 0x00 0x03 0x00
CRC:           0xCA 0x95
```

Hexadecimal: `02 04 06 01 00 02 00 03 00 CA 95`

**Example: Write Single Coil (Function 05)**

Request to turn ON a coil at address 0x0010 of device 3:

```
Slave Address: 0x03
Function Code: 0x05
Coil Address:  0x0010
Value:         0xFF 0x00 (ON = 0xFF00, OFF = 0x0000)
CRC:           0x8D 0x78
```

Hexadecimal: `03 05 00 10 FF 00 8D 78`

Response (echo of request):

```
Slave Address: 0x03
Function Code: 0x05
Coil Address:  0x0010
Value:         0xFF 0x00
CRC:           0x8D 0x78
```

Hexadecimal: `03 05 00 10 FF 00 8D 78`

### 3. ASCII Packager

ASCII frames use printable ASCII characters with LRC checksum.

**ASCII Frame Format:**

| Field | Length | Description |
|-------|--------|-------------|
| Start delimiter | 1 char | Colon character (`:`) |
| Payload | 2N chars | Hex ASCII representation of address, PDU, and LRC |
| End delimiters | 2 chars | Carriage return + line feed (`\r\n`) |

**Example: Read Holding Registers (Function 03)**

Request to read 2 registers from address 0x0100 of device 1:

```
Start:          `:` 
Slave Address:  `01` (hex ASCII)
Function Code:  `03` (hex ASCII)
Start Address:  `0100` (hex ASCII)
Quantity:       `0002` (hex ASCII)
LRC:            `FA` (hex ASCII)
End:            `\r\n`
```

ASCII format: `:010301000002FA\r\n`

Response with values 0x0064 and 0x00C8:

```
Start:          `:`
Slave Address:  `01` (hex ASCII)
Function Code:  `03` (hex ASCII)
Byte Count:     `04` (hex ASCII)
Data:           `00640064` (hex ASCII)
LRC:            `8A` (hex ASCII)
End:            `\r\n`
```

ASCII format: `:0103040064006489\r\n`

**Example: Write Multiple Coils (Function 15)**

Request to write 10 coils starting at address 0x0030 of device 5:

```
Start:          `:`
Slave Address:  `05` (hex ASCII)
Function Code:  `0F` (hex ASCII)
Start Address:  `0030` (hex ASCII)
Quantity:       `000A` (hex ASCII)
Byte Count:     `02` (hex ASCII)
Data:           `CD01` (coils status, binary 11001101 00000001)
LRC:            `12` (hex ASCII)
End:            `\r\n`
```

ASCII format: `:050F0030000A02CD0112\r\n`

Response confirming write:

```
Start:          `:`
Slave Address:  `05` (hex ASCII)
Function Code:  `0F` (hex ASCII)
Start Address:  `0030` (hex ASCII)
Quantity:       `000A` (hex ASCII)
LRC:            `D2` (hex ASCII)
End:            `\r\n`
```

ASCII format: `:050F0030000AD2\r\n`

## Common PDU Formats

The Protocol Data Unit (PDU) format is independent of the packager used and follows these patterns:

### Request PDU Formats

| Function | Code | Format | Example (hex) |
|----------|------|--------|---------------|
| Read Coils | 0x01 | Function (1) + Start Address (2) + Quantity (2) | `01 00 30 00 0A` |
| Read Discrete Inputs | 0x02 | Function (1) + Start Address (2) + Quantity (2) | `02 00 30 00 0A` |
| Read Holding Registers | 0x03 | Function (1) + Start Address (2) + Quantity (2) | `03 01 00 00 02` |
| Read Input Registers | 0x04 | Function (1) + Start Address (2) + Quantity (2) | `04 08 00 00 03` |
| Write Single Coil | 0x05 | Function (1) + Address (2) + Value (2) | `05 00 10 FF 00` |
| Write Single Register | 0x06 | Function (1) + Address (2) + Value (2) | `06 02 00 00 64` |
| Write Multiple Coils | 0x0F | Function (1) + Start Address (2) + Quantity (2) + Byte Count (1) + Data (N) | `0F 00 30 00 0A 02 CD 01` |
| Write Multiple Registers | 0x10 | Function (1) + Start Address (2) + Quantity (2) + Byte Count (1) + Data (N) | `10 02 00 00 02 04 00 64 00 C8` |

### Response PDU Formats

| Function | Code | Format | Example (hex) |
|----------|------|--------|---------------|
| Read Coils | 0x01 | Function (1) + Byte Count (1) + Data (N) | `01 02 C5 01` |
| Read Discrete Inputs | 0x02 | Function (1) + Byte Count (1) + Data (N) | `02 02 C5 01` |
| Read Holding Registers | 0x03 | Function (1) + Byte Count (1) + Data (N) | `03 04 00 64 00 C8` |
| Read Input Registers | 0x04 | Function (1) + Byte Count (1) + Data (N) | `04 06 01 00 02 00 03 00` |
| Write Single Coil | 0x05 | Function (1) + Address (2) + Value (2) | `05 00 10 FF 00` |
| Write Single Register | 0x06 | Function (1) + Address (2) + Value (2) | `06 02 00 00 64` |
| Write Multiple Coils | 0x0F | Function (1) + Start Address (2) + Quantity (2) | `0F 00 30 00 0A` |
| Write Multiple Registers | 0x10 | Function (1) + Start Address (2) + Quantity (2) | `10 02 00 00 02` |

### Exception Response

For all functions, exception responses use the function code with the MSB set (code + 0x80):

| Field | Length | Description | Example |
|-------|--------|-------------|---------|
| Exception Function | 1 byte | Original function code + 0x80 | 0x83 (Read Holding Registers + 0x80) |
| Exception Code | 1 byte | Error code (1-4 typically) | 0x02 (Illegal Data Address) |

## Using Packagers

Each transport type automatically provides its most appropriate packager:

- TCP transports will provide MBAP packager
- UART transports will provide RTU packager

```cpp
// Let the transport choose the default packager
auto tcpTransport = std::make_shared<ModbusTcpTransport>("192.168.1.100", 502);
auto tcpMaster = std::make_shared<ModbusMaster>(tcpTransport);  // Uses MBAP by default

auto uartTransport = std::make_shared<ModbusUartTransport>(UART_NUM_1, GPIO_NUM_17, GPIO_NUM_16);
auto rtuMaster = std::make_shared<ModbusMaster>(uartTransport); // Uses RTU by default
```

### Explicit Packager Selection

You can explicitly specify a custom packager in the ModbusConfig when you need non-standard behavior:

```cpp
// Create a custom packager
auto asciiPackager = ModbusPackagerFactory::createAsciiPackager();

// Configure and create master with custom packager
ModbusConfig config;
config.packager = asciiPackager;
auto master = std::make_shared<ModbusMaster>(transport, config);
```

### Runtime Packager Switching

You can change the packager at runtime:

```cpp
// Switch to ASCII packager
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
#include "modbus_uart_transport.hpp"
#include "esp_log.h"

namespace speed {
namespace net {
namespace modbus {

static const char* TAG = "ModbusUartTransport";

ModbusUartTransport::ModbusUartTransport(uart_port_t uart_num, int tx_pin, int rx_pin, int rts_pin, int baud_rate)
    : _uart_num(uart_num), _tx_pin(tx_pin), _rx_pin(rx_pin), _rts_pin(rts_pin), 
      _baud_rate(baud_rate), _initialized(false) {}

ModbusUartTransport::~ModbusUartTransport() {
    stop();
}

bool ModbusUartTransport::begin() {
    uart_config_t uart_config = {
        .baud_rate = _baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    esp_err_t err = uart_param_config(_uart_num, &uart_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure UART parameters");
        return false;
    }

    err = uart_set_pin(_uart_num, _tx_pin, _rx_pin, _rts_pin, UART_PIN_NO_CHANGE);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set UART pins");
        return false;
    }

    err = uart_driver_install(_uart_num, MAX_FRAME_SIZE * 2, 0, 0, NULL, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install UART driver");
        return false;
    }

    _initialized = true;
    return true;
}

void ModbusUartTransport::stop() {
    if (_initialized) {
        uart_driver_delete(_uart_num);
        _initialized = false;
    }
}

bool ModbusUartTransport::send(const uint8_t* data, size_t length) {
    if (!_initialized) return false;
    int txBytes = uart_write_bytes(_uart_num, (const char*)data, length);
    return txBytes == (int)length;
}

bool ModbusUartTransport::receive(uint8_t* buffer, size_t expected_length, uint32_t timeout_ms) {
    if (!_initialized) return false;

    int totalBytes = 0;
    int remaining = expected_length;
    uint32_t startTime = xTaskGetTickCount();
    
    while (remaining > 0) {
        int len = uart_read_bytes(_uart_num, buffer + totalBytes, remaining, pdMS_TO_TICKS(10));
        if (len > 0) {
            totalBytes += len;
            remaining -= len;
        }
        if ((xTaskGetTickCount() - startTime) > pdMS_TO_TICKS(timeout_ms))
            break;
    }
    
    if (totalBytes != (int)expected_length) {
        ESP_LOGW(TAG, "Expected %d bytes but received %d", expected_length, totalBytes);
        return false;
    }
    return true;
}

void ModbusUartTransport::flush() {
    if (_initialized) {
        uart_flush(_uart_num);
    }
}

} // namespace modbus
} // namespace net
} // namespace speed
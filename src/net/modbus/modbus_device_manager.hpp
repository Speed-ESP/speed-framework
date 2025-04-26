#pragma once

#include <memory>
#include <map>
#include <vector>
#include <functional>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <net/modbus/modbus_master.hpp>
#include <net/modbus/modbus_client.hpp>

namespace speed
{
    namespace net
    {
        namespace modbus
        {

            // Device discovery callback
            using DeviceDiscoveryCallback = std::function<void(uint8_t address, bool present)>;

            class ModbusDeviceManager
            {
            public:
                explicit ModbusDeviceManager(std::shared_ptr<ModbusMaster> master);

                // Device management
                std::shared_ptr<ModbusClient> addDevice(uint8_t address, const std::string &name = "");
                void removeDevice(uint8_t address);
                std::shared_ptr<ModbusClient> getDevice(uint8_t address);
                std::vector<std::shared_ptr<ModbusClient>> getAllDevices() const;

                // Network scanning
                void scanNetwork(uint8_t startAddr, uint8_t endAddr, DeviceDiscoveryCallback callback);
                void stopScan();

                // Network statistics
                const ModbusStatistics &getNetworkStatistics() const;
                void resetNetworkStatistics();

                // Network management
                void disconnectAll();
                size_t getDeviceCount() const { return _devices.size(); }
                bool hasDevice(uint8_t address) const { return _devices.find(address) != _devices.end(); }

            private:
                std::shared_ptr<ModbusMaster> _master;
                std::map<uint8_t, std::shared_ptr<ModbusClient>> _devices;
                bool _scanning{false};
                TaskHandle_t _scanTask{nullptr};

                static void scanTaskFunction(void *param);
                void processScanTask(uint8_t startAddr, uint8_t endAddr, DeviceDiscoveryCallback callback);
                bool testDevicePresence(uint8_t address);
            };

        } // namespace modbus
    } // namespace net
} // namespace speed
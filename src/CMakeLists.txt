# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

# Define variables for each folder
set(NET_SOURCES "")
set(MQTT_SOURCES "")
set(MODBUS_SOURCE "")
set(CORE_SOURCES "")
set(COM_SOURCES "")
set(BLE_SOURCES "")
set(SETTINGS_SOURCES "")
set(ENTRY_SOURCE "")
# Dynamically include sources from specific folders
# MQTT sources: only include if enabled in KConfig


if(DEFINED CONFIG_SPEED_FRAMEWORK_USE_NETWORK AND CONFIG_SPEED_FRAMEWORK_USE_NETWORK)
    FILE(GLOB_RECURSE NET_SOURCES ${NET_SOURCES} ${CMAKE_SOURCE_DIR}/src/net/speed_net.*)
endif()

if(DEFINED CONFIG_SPEED_FRAMEWORK_USE_MQTT AND SPEED_FRAMEWORK_USE_MQTT)
    FILE(GLOB_RECURSE MQTT_SOURCES ${CMAKE_SOURCE_DIR}/src/mqtt/*.*)
endif()

if(DEFINED CONFIG_SPEED_FRAMEWORK_USE_MODBUS AND CONFIG_SPEED_FRAMEWORK_USE_MODBUS)
    FILE(GLOB_RECURSE MODBUS_SOURCE ${CMAKE_SOURCE_DIR}/src/net/modbus/*.*)
endif()
# CORE sources: always include
FILE(GLOB_RECURSE CORE_SOURCES ${CMAKE_SOURCE_DIR}/src/core/*.*)

# COM sources: if the folder exists, glob its files; otherwise, create an empty variable
if(EXISTS "${CMAKE_SOURCE_DIR}/src/com")
    FILE(GLOB_RECURSE COM_SOURCES ${CMAKE_SOURCE_DIR}/src/com/*.*)
else()
    set(COM_SOURCES "")
endif()

# BLE sources: include only if BLE is enabled in KConfig
if(DEFINED CONFIG_SPEED_FRAMEWORK_USE_BLE AND CONFIG_SPEED_FRAMEWORK_USE_BLE)
    FILE(GLOB_RECURSE BLE_SOURCES ${BLE_SOURCES} ${CMAKE_SOURCE_DIR}/src/ble/speed_ble.*)
endif()

if(DEFINED CONFIG_SPEED_FRAMEWORK_USE_BLUFI AND CONFIG_SPEED_FRAMEWORK_USE_BLUFI)
    FILE(GLOB_RECURSE BLE_SOURCES ${BLE_SOURCES} ${CMAKE_SOURCE_DIR}/src/ble/blufi.*)
endif()

# SETTINGS sources: if the folder exists, glob its files; otherwise, create an empty variable
if(EXISTS "${CMAKE_SOURCE_DIR}/src/settings")
    FILE(GLOB_RECURSE SETTINGS_SOURCES ${CMAKE_SOURCE_DIR}/src/settings/*.*)
else()
    set(SETTINGS_SOURCES "")
endif()


FILE(GLOB_RECURSE ENTRY_SOURCE ${CMAKE_SOURCE_DIR}/src/main.cpp)

# Combine all sources into app_sources
set(app_sources 
    ${NET_SOURCES} 
    ${MQTT_SOURCES}
    ${MODBUS_SOURCE}
    ${CORE_SOURCES} 
    ${COM_SOURCES} 
    ${BLE_SOURCES} 
    ${SETTINGS_SOURCES} 
    ${ENTRY_SOURCE})

idf_component_register(SRCS ${app_sources} REQUIRES esp_wifi)
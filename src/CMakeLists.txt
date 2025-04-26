# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

# Define variables for each folder
set(NET_SOURCES "")
set(CORE_SOURCES "")
set(COM_SOURCES "")
set(BLE_SOURCES "")
set(SETTINGS_SOURCES "")

# Dynamically include sources from specific folders
# MQTT sources: only include if enabled in KConfig
if(DEFINED SPEED_FRAMEWORK_USE_MQTT AND SPEED_FRAMEWORK_USE_MQTT)
    FILE(GLOB_RECURSE NET_SOURCES ${CMAKE_SOURCE_DIR}/src/net/*.*)
else()
    set(NET_SOURCES "")
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
if(DEFINED SPEED_FRAMEWORK_USE_BLE AND SPEED_FRAMEWORK_USE_BLE)
    FILE(GLOB_RECURSE BLE_SOURCES ${CMAKE_SOURCE_DIR}/src/ble/*.*)
else()
    set(BLE_SOURCES "")
endif()

# SETTINGS sources: if the folder exists, glob its files; otherwise, create an empty variable
if(EXISTS "${CMAKE_SOURCE_DIR}/src/settings")
    FILE(GLOB_RECURSE SETTINGS_SOURCES ${CMAKE_SOURCE_DIR}/src/settings/*.*)
else()
    set(SETTINGS_SOURCES "")
endif()



# Combine all sources into app_sources
set(app_sources ${NET_SOURCES} ${CORE_SOURCES} ${COM_SOURCES} ${BLE_SOURCES} ${SETTINGS_SOURCES} ${CMAKE_SOURCE_DIR}/src/main.cpp)

idf_component_register(SRCS ${app_sources})
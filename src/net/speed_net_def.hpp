#pragma once

#include <SmartEnumCpp/SmartEnum.hpp>
#include <esp_wifi_types_generic.h>

namespace speed::net
{

    /**
     * @brief WiFi operation mode enumeration
     *
     * Represents the different WiFi operating modes supported by ESP32.
     * Implemented as a SmartEnum for type safety and better error handling.
     */
    class SpeedWifiMode : public SmartEnum<SpeedWifiMode, wifi_mode_t>
    {
    private:
        wifi_mode_t _value;
        std::string _name;
        SpeedWifiMode(const std::string &name, wifi_mode_t value) : SmartEnum(name, value) {}

    public:
        /** @brief Disabled mode */
        static const SpeedWifiMode NULL_MODE;
        /** @brief Station (client) mode */
        static const SpeedWifiMode STA;
        /** @brief Access Point mode */
        static const SpeedWifiMode AP;
        /** @brief Concurrent AP and station mode */
        static const SpeedWifiMode APSTA;
    };

    /** @brief Disabled mode */
    inline const SpeedWifiMode SpeedWifiMode::NULL_MODE("NULL", WIFI_MODE_NULL);
    /** @brief Station (client) mode */
    inline const SpeedWifiMode SpeedWifiMode::STA("STA", WIFI_MODE_STA);
    /** @brief Access Point mode */
    inline const SpeedWifiMode SpeedWifiMode::AP("AP", WIFI_MODE_AP);
    /** @brief Concurrent AP and station mode */
    inline const SpeedWifiMode SpeedWifiMode::APSTA("APSTA", WIFI_MODE_APSTA);

    /**
     * @brief WiFi authentication mode enumeration
     *
     * Represents the different WiFi authentication modes supported by ESP32.
     * Implemented as a SmartEnum for type safety and better error handling.
     */
    class SpeedWifiAuthMode : public SmartEnum<SpeedWifiAuthMode, wifi_auth_mode_t>
    {
    private:
        SpeedWifiAuthMode(const std::string &name, wifi_auth_mode_t value) : SmartEnum(name, value) {}

    public:
        /** @brief Open network (no authentication) */
        static const SpeedWifiAuthMode OPEN;
        /** @brief WEP authentication */
        static const SpeedWifiAuthMode WEP;
        /** @brief WPA-PSK authentication */
        static const SpeedWifiAuthMode WPA_PSK;
        /** @brief WPA2-PSK authentication */
        static const SpeedWifiAuthMode WPA2_PSK;
        /** @brief WPA/WPA2-PSK mixed mode authentication */
        static const SpeedWifiAuthMode WPA_WPA2_PSK;
        /** @brief WPA2 enterprise authentication */
        static const SpeedWifiAuthMode WPA2_ENTERPRISE;
        /** @brief WPA3-PSK authentication */
        static const SpeedWifiAuthMode WPA3_PSK;
        /** @brief WPA2/WPA3-PSK mixed mode authentication */
        static const SpeedWifiAuthMode WPA2_WPA3_PSK;
    };

    // Define the static members
    inline const SpeedWifiAuthMode SpeedWifiAuthMode::OPEN("OPEN", WIFI_AUTH_OPEN);
    inline const SpeedWifiAuthMode SpeedWifiAuthMode::WEP("WEP", WIFI_AUTH_WEP);
    inline const SpeedWifiAuthMode SpeedWifiAuthMode::WPA_PSK("WPA_PSK", WIFI_AUTH_WPA_PSK);
    inline const SpeedWifiAuthMode SpeedWifiAuthMode::WPA2_PSK("WPA2_PSK", WIFI_AUTH_WPA2_PSK);
    inline const SpeedWifiAuthMode SpeedWifiAuthMode::WPA_WPA2_PSK("WPA_WPA2_PSK", WIFI_AUTH_WPA_WPA2_PSK);
    inline const SpeedWifiAuthMode SpeedWifiAuthMode::WPA2_ENTERPRISE("WPA2_ENTERPRISE", WIFI_AUTH_WPA2_ENTERPRISE);
    inline const SpeedWifiAuthMode SpeedWifiAuthMode::WPA3_PSK("WPA3_PSK", WIFI_AUTH_WPA3_PSK);
    inline const SpeedWifiAuthMode SpeedWifiAuthMode::WPA2_WPA3_PSK("WPA2_WPA3_PSK", WIFI_AUTH_WPA2_WPA3_PSK);

    /**
     * @brief WiFi scan method enumeration
     *
     * Defines different WiFi scanning methods supported by ESP32.
     */
    class SpeedWifiScanMethod : public SmartEnum<SpeedWifiScanMethod, wifi_scan_method_t>
    {
    private:
        SpeedWifiScanMethod(const std::string &name, wifi_scan_method_t value) : SmartEnum(name, value) {}

    public:
        /** @brief Fast scan method that scans only the target channel */
        static const SpeedWifiScanMethod FAST;
        /** @brief Comprehensive scan that checks all channels */
        static const SpeedWifiScanMethod ALL_CHANNEL;
    };

    // Define the static members
    inline const SpeedWifiScanMethod SpeedWifiScanMethod::FAST("FAST", WIFI_FAST_SCAN);
    inline const SpeedWifiScanMethod SpeedWifiScanMethod::ALL_CHANNEL("ALL_CHANNEL", WIFI_ALL_CHANNEL_SCAN);

    /**
     * @brief WiFi scan type enumeration
     *
     * Defines the scan types (active or passive) for WiFi network scanning.
     * Active scanning sends probe requests, while passive scanning just listens.
     */
    class SpeedWifiScanType : public SmartEnum<SpeedWifiScanType, wifi_scan_type_t>
    {
    private:
        SpeedWifiScanType(const std::string &name, wifi_scan_type_t value) : SmartEnum(name, value) {}

    public:
        /** @brief Active scanning that sends probe requests */
        static const SpeedWifiScanType ACTIVE;
        /** @brief Passive scanning that only listens for beacons */
        static const SpeedWifiScanType PASSIVE;
    };

    // Define the static members
    inline const SpeedWifiScanType SpeedWifiScanType::ACTIVE("ACTIVE", WIFI_SCAN_TYPE_ACTIVE);
    inline const SpeedWifiScanType SpeedWifiScanType::PASSIVE("PASSIVE", WIFI_SCAN_TYPE_PASSIVE);

    /**
     * @brief WiFi connection sort method enumeration
     *
     * Defines the methods used by ESP32 to sort and prioritize WiFi access points
     * when connecting.
     */
    class SpeedWifiSortMethod : public SmartEnum<SpeedWifiSortMethod, wifi_sort_method_t>
    {
    private:
        SpeedWifiSortMethod(const std::string &name, wifi_sort_method_t value) : SmartEnum(name, value) {}

    public:
        /** @brief Sort access points by signal strength (RSSI) */
        static const SpeedWifiSortMethod BY_RSSI;
        /** @brief Sort access points by security level */
        static const SpeedWifiSortMethod BY_SECURITY;
    };

    // Define the static members
    inline const SpeedWifiSortMethod SpeedWifiSortMethod::BY_RSSI("BY_RSSI", WIFI_CONNECT_AP_BY_SIGNAL);
    inline const SpeedWifiSortMethod SpeedWifiSortMethod::BY_SECURITY("BY_SECURITY", WIFI_CONNECT_AP_BY_SECURITY);

    /**
     * @brief WiFi connection status enumeration
     *
     * Represents the different states a WiFi connection can be in.
     * This enum provides helper methods to check the connection state.
     */
    class SpeedWifiConnectionStatus : public SmartEnum<SpeedWifiConnectionStatus, uint8_t>
    {
    private:
        SpeedWifiConnectionStatus(const std::string &name, uint8_t value) : SmartEnum(name, value) {}

    public:
        /** @brief Initial state before attempting connection */
        static const SpeedWifiConnectionStatus IDLE;
        /** @brief Currently attempting to connect */
        static const SpeedWifiConnectionStatus CONNECTING;
        /** @brief Successfully connected */
        static const SpeedWifiConnectionStatus CONNECTED;
        /** @brief Disconnected from a previous connection */
        static const SpeedWifiConnectionStatus DISCONNECTED;
        /** @brief Connection attempt failed */
        static const SpeedWifiConnectionStatus FAILED;

        /**
         * @brief Check if connection is established
         * @return true if connected, false otherwise
         */
        bool isConnected() const { return *this == CONNECTED; }

        /**
         * @brief Check if device is not connected
         * @return true if disconnected, idle, or failed, false otherwise
         */
        bool isDisconnected() const { return *this == DISCONNECTED || *this == IDLE || *this == FAILED; }

        /**
         * @brief Check if connection is in progress
         * @return true if connecting, false otherwise
         */
        bool isTransitioning() const { return *this == CONNECTING; }
    };

    // Define the static members
    inline const SpeedWifiConnectionStatus SpeedWifiConnectionStatus::IDLE("IDLE", 0);
    inline const SpeedWifiConnectionStatus SpeedWifiConnectionStatus::CONNECTING("CONNECTING", 1);
    inline const SpeedWifiConnectionStatus SpeedWifiConnectionStatus::CONNECTED("CONNECTED", 2);
    inline const SpeedWifiConnectionStatus SpeedWifiConnectionStatus::DISCONNECTED("DISCONNECTED", 3);
    inline const SpeedWifiConnectionStatus SpeedWifiConnectionStatus::FAILED("FAILED", 4);

    /**
     * @brief WiFi disconnect reason enumeration with descriptive properties
     *
     * Advanced implementation of disconnect reasons that includes human-readable
     * descriptions for each reason. This helps with debugging and logging.
     * Each enum instance is a full object with its own description property.
     */
    class SpeedWifiDisconnectReason : public SmartEnum<SpeedWifiDisconnectReason, uint8_t>
    {
    private:
        std::string _description;

        SpeedWifiDisconnectReason(const std::string &name, uint8_t value, const std::string &description)
            : SmartEnum(name, value)
        {
            _description = description;
        }

    public:
        // Explicitly delete copy constructor to match base class behavior
        SpeedWifiDisconnectReason(const SpeedWifiDisconnectReason &other) = delete;
        /**
         * @brief Get a human-readable description of the disconnect reason
         * @return std::string Description of the disconnect reason
         */
        std::string getDescription() const { return _description; }

        // All disconnect reason instances with their descriptions
        /** @brief Unspecified reason */
        static const SpeedWifiDisconnectReason UNSPECIFIED;
        /** @brief Authentication expired */
        static const SpeedWifiDisconnectReason AUTH_EXPIRE;
        /** @brief Authentication leave */
        static const SpeedWifiDisconnectReason AUTH_LEAVE;
        /** @brief Association expired */
        static const SpeedWifiDisconnectReason ASSOC_EXPIRE;
        /** @brief Too many stations associated */
        static const SpeedWifiDisconnectReason ASSOC_TOOMANY;
        /** @brief Not authenticated */
        static const SpeedWifiDisconnectReason NOT_AUTHED;
        /** @brief Not associated */
        static const SpeedWifiDisconnectReason NOT_ASSOCED;
        /** @brief Association leave */
        static const SpeedWifiDisconnectReason ASSOC_LEAVE;
        /** @brief Association not authenticated */
        static const SpeedWifiDisconnectReason ASSOC_NOT_AUTHED;
        /** @brief Disassociated due to bad power capability */
        static const SpeedWifiDisconnectReason DISASSOC_PWRCAP_BAD;
        /** @brief Disassociated due to bad supported channels */
        static const SpeedWifiDisconnectReason DISASSOC_SUPCHAN_BAD;
        /** @brief Invalid information element */
        static const SpeedWifiDisconnectReason IE_INVALID;
        /** @brief Message integrity code failure */
        static const SpeedWifiDisconnectReason MIC_FAILURE;
        /** @brief 4-way handshake timeout */
        static const SpeedWifiDisconnectReason GROUP_KEY_UPDATE_TIMEOUT;
        /** @brief Invalid group cipher */
        static const SpeedWifiDisconnectReason GROUP_CIPHER_INVALID;
        /** @brief Invalid pairwise cipher */
        static const SpeedWifiDisconnectReason PAIRWISE_CIPHER_INVALID;
        /** @brief Invalid authentication key management protocol */
        static const SpeedWifiDisconnectReason AKMP_INVALID;
        /** @brief Unsupported RSN IE version */
        static const SpeedWifiDisconnectReason UNSUPP_RSN_IE_VERSION;
        /** @brief Invalid RSN IE capabilities */
        static const SpeedWifiDisconnectReason INVALID_RSN_IE_CAP;
        /** @brief IEEE 802.1X authentication failed */
        static const SpeedWifiDisconnectReason REASON_802_1X_AUTH_FAILED;
        /** @brief Cipher suite rejected */
        static const SpeedWifiDisconnectReason CIPHER_SUITE_REJECTED;
        /** @brief Beacon timeout */
        static const SpeedWifiDisconnectReason BEACON_TIMEOUT;
        /** @brief No access point found */
        static const SpeedWifiDisconnectReason NO_AP_FOUND;
        /** @brief Authentication failed */
        static const SpeedWifiDisconnectReason AUTH_FAIL;
        /** @brief Association failed */
        static const SpeedWifiDisconnectReason ASSOC_FAIL;
        /** @brief Handshake timeout */
        static const SpeedWifiDisconnectReason HANDSHAKE_TIMEOUT;
        /** @brief Connection failed */
        static const SpeedWifiDisconnectReason CONNECTION_FAIL;
    };

    // Define the static members with their descriptions
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::UNSPECIFIED("UNSPECIFIED", WIFI_REASON_UNSPECIFIED, "Unspecified reason");
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::AUTH_EXPIRE("AUTH_EXPIRE", WIFI_REASON_AUTH_EXPIRE, "Authentication expired");
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::AUTH_LEAVE("AUTH_LEAVE", WIFI_REASON_AUTH_LEAVE, "Authentication leave");
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::ASSOC_EXPIRE("ASSOC_EXPIRE", WIFI_REASON_ASSOC_EXPIRE, "Association expired");
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::ASSOC_TOOMANY("ASSOC_TOOMANY", WIFI_REASON_ASSOC_TOOMANY, "Too many stations associated");
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::NOT_AUTHED("NOT_AUTHED", WIFI_REASON_NOT_AUTHED, "Not authenticated");
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::NOT_ASSOCED("NOT_ASSOCED", WIFI_REASON_NOT_ASSOCED, "Not associated");
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::ASSOC_LEAVE("ASSOC_LEAVE", WIFI_REASON_ASSOC_LEAVE, "Association leave");
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::ASSOC_NOT_AUTHED("ASSOC_NOT_AUTHED", WIFI_REASON_ASSOC_NOT_AUTHED, "Association not authenticated");
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::DISASSOC_PWRCAP_BAD("DISASSOC_PWRCAP_BAD", WIFI_REASON_DISASSOC_PWRCAP_BAD, "Disassociated due to bad power capability");
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::DISASSOC_SUPCHAN_BAD("DISASSOC_SUPCHAN_BAD", WIFI_REASON_DISASSOC_SUPCHAN_BAD, "Disassociated due to bad supported channels");
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::IE_INVALID("IE_INVALID", WIFI_REASON_IE_INVALID, "Invalid information element");
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::MIC_FAILURE("MIC_FAILURE", WIFI_REASON_MIC_FAILURE, "Message integrity code failure");
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::GROUP_KEY_UPDATE_TIMEOUT("GROUP_KEY_UPDATE_TIMEOUT", WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT, "4-way handshake timeout");
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::GROUP_CIPHER_INVALID("GROUP_CIPHER_INVALID", WIFI_REASON_GROUP_CIPHER_INVALID, "Invalid group cipher");
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::PAIRWISE_CIPHER_INVALID("PAIRWISE_CIPHER_INVALID", WIFI_REASON_PAIRWISE_CIPHER_INVALID, "Invalid pairwise cipher");
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::AKMP_INVALID("AKMP_INVALID", WIFI_REASON_AKMP_INVALID, "Invalid authentication key management protocol");
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::UNSUPP_RSN_IE_VERSION("UNSUPP_RSN_IE_VERSION", WIFI_REASON_UNSUPP_RSN_IE_VERSION, "Unsupported RSN IE version");
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::INVALID_RSN_IE_CAP("INVALID_RSN_IE_CAP", WIFI_REASON_INVALID_RSN_IE_CAP, "Invalid RSN IE capabilities");
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::REASON_802_1X_AUTH_FAILED("REASON_802_1X_AUTH_FAILED", WIFI_REASON_802_1X_AUTH_FAILED, "IEEE 802.1X authentication failed");
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::CIPHER_SUITE_REJECTED("CIPHER_SUITE_REJECTED", WIFI_REASON_CIPHER_SUITE_REJECTED, "Cipher suite rejected");
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::BEACON_TIMEOUT("BEACON_TIMEOUT", WIFI_REASON_BEACON_TIMEOUT, "Beacon timeout");
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::NO_AP_FOUND("NO_AP_FOUND", WIFI_REASON_NO_AP_FOUND, "No access point found");
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::AUTH_FAIL("AUTH_FAIL", WIFI_REASON_AUTH_FAIL, "Authentication failed");
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::ASSOC_FAIL("ASSOC_FAIL", WIFI_REASON_ASSOC_FAIL, "Association failed");
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::HANDSHAKE_TIMEOUT("HANDSHAKE_TIMEOUT", WIFI_REASON_HANDSHAKE_TIMEOUT, "Handshake timeout");
    inline const SpeedWifiDisconnectReason SpeedWifiDisconnectReason::CONNECTION_FAIL("CONNECTION_FAIL", WIFI_REASON_CONNECTION_FAIL, "Connection failed");

}

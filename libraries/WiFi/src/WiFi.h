/*
 * WiFi.h - WiFi del W800/W801/W802/Air101 con la interfaz del ESP32.
 *
 *   WiFi.begin("MiRed", "clave");
 *   while (WiFi.status() != WL_CONNECTED) delay(500);
 *   Serial.println(WiFi.localIP());
 *
 * Por debajo usa el gestor de red del WM IoT SDK (conexion, DHCP y
 * reconexion automatica) y lwIP. W806 y Air103 no tienen radio.
 *
 * Parte de w80x_arduino. LGPL 2.1.
 */
#ifndef WiFi_h
#define WiFi_h

#include <Arduino.h>
#include "wmsdk_config.h"

#if !defined(CONFIG_CHIP_HAVE_WIFI) || !CONFIG_CHIP_HAVE_WIFI
#error "Esta placa no tiene WiFi (W806/Air103). Elija W800, W801, W802 o Air101."
#endif

#include "IPAddress.h"
#include "WiFiType.h"
#include "WiFiClient.h"
#include "WiFiServer.h"
#include "WiFiUdp.h"

typedef void (*WiFiEventCb)(int event);

/* eventos para WiFi.onEvent() */
enum {
    ARDUINO_EVENT_WIFI_STA_CONNECTED = 1,
    ARDUINO_EVENT_WIFI_STA_DISCONNECTED,
    ARDUINO_EVENT_WIFI_STA_GOT_IP,
    ARDUINO_EVENT_WIFI_STA_LOST_IP,
    ARDUINO_EVENT_WIFI_AP_START,
    ARDUINO_EVENT_WIFI_AP_STACONNECTED,
    ARDUINO_EVENT_WIFI_AP_STADISCONNECTED,
    ARDUINO_EVENT_WIFI_SCAN_DONE,
};

class WiFiClass
{
public:
    WiFiClass();

    /* ---- estacion ---- */
    wl_status_t begin(const char *ssid, const char *passphrase = NULL, int32_t channel = 0, const uint8_t *bssid = NULL,
                      bool connect = true);
    wl_status_t begin(const String &ssid, const String &passphrase) { return begin(ssid.c_str(), passphrase.c_str()); }
    wl_status_t begin(); /* con la ultima red usada */
    bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1 = (uint32_t)0,
                IPAddress dns2 = (uint32_t)0);
    bool disconnect(bool wifioff = false, bool eraseap = false);
    bool reconnect();
    bool isConnected() { return status() == WL_CONNECTED; }
    wl_status_t status();
    uint8_t waitForConnectResult(unsigned long timeoutLength = 60000);
    bool setAutoReconnect(bool autoReconnect) { _autoReconnect = autoReconnect; return true; }
    bool getAutoReconnect() { return _autoReconnect; }
    bool setHostname(const char *hostname);
    const char *getHostname() { return _hostname; }

    IPAddress localIP();
    IPAddress subnetMask();
    IPAddress gatewayIP();
    IPAddress dnsIP(uint8_t dns_no = 0);
    IPAddress broadcastIP();
    uint8_t *macAddress(uint8_t *mac);
    String macAddress();
    String SSID();
    String psk() { return String(_pass); }
    uint8_t *BSSID();
    String BSSIDstr();
    int8_t RSSI();
    int32_t channel();

    /* ---- punto de acceso ---- */
    bool softAP(const char *ssid, const char *passphrase = NULL, int channel = 1, int ssid_hidden = 0,
                int max_connection = 8);
    bool softAP(const String &ssid, const String &passphrase = String()) { return softAP(ssid.c_str(), passphrase.length() ? passphrase.c_str() : NULL); }
    bool softAPdisconnect(bool wifioff = false);
    IPAddress softAPIP();
    IPAddress softAPSubnetMask();
    uint8_t softAPgetStationNum();
    String softAPmacAddress();
    uint8_t *softAPmacAddress(uint8_t *mac);
    String softAPSSID() { return String(_apSsid); }

    /* ---- modo ---- */
    bool mode(wifi_mode_t m);
    wifi_mode_t getMode() { return _mode; }
    bool enableSTA(bool enable);
    bool enableAP(bool enable);

    /* ---- busqueda de redes ---- */
    int16_t scanNetworks(bool async = false, bool show_hidden = false);
    int16_t scanComplete() { return _scanCount; }
    void scanDelete();
    String SSID(uint8_t i);
    int32_t RSSI(uint8_t i);
    uint8_t encryptionType(uint8_t i);
    int32_t channel(uint8_t i);
    uint8_t *BSSID(uint8_t i);
    String BSSIDstr(uint8_t i);

    /* ---- DNS ---- */
    int hostByName(const char *host, IPAddress &result);

    /* ---- eventos ---- */
    void onEvent(WiFiEventCb cb) { _eventCb = cb; }

    /* uso interno */
    void _onWifiEvent(int event, void *data);
    void _onNmEvent(int event, void *data);

private:
    bool initEvents();
    void applyStaticIP();
    wifi_mode_t _mode;
    volatile wl_status_t _status;
    bool _autoReconnect;
    bool _events;
    bool _staStarted, _apStarted;
    bool _static;
    IPAddress _ip, _gw, _mask, _dns1, _dns2;
    char _ssid[33];
    char _pass[65];
    char _apSsid[33];
    char _apPass[65];
    char _hostname[33];
    uint8_t _bssid[6];
    int16_t _scanCount;
    void *_scan;
    WiFiEventCb _eventCb;
};

extern WiFiClass WiFi;

#endif

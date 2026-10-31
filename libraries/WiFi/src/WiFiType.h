/*
 * WiFiType.h - constantes de WiFi (mismos nombres que en Arduino/ESP32).
 * Parte de w80x_arduino. LGPL 2.1.
 */
#ifndef WIFI_TYPE_H
#define WIFI_TYPE_H

typedef enum {
    WL_NO_SHIELD       = 255,
    WL_IDLE_STATUS     = 0,
    WL_NO_SSID_AVAIL   = 1,
    WL_SCAN_COMPLETED  = 2,
    WL_CONNECTED       = 3,
    WL_CONNECT_FAILED  = 4,
    WL_CONNECTION_LOST = 5,
    WL_DISCONNECTED    = 6
} wl_status_t;

typedef enum {
    WIFI_OFF    = 0,
    WIFI_STA    = 1,
    WIFI_AP     = 2,
    WIFI_AP_STA = 3,
} wifi_mode_t;
#define WIFI_MODE_NULL  WIFI_OFF
#define WIFI_MODE_STA   WIFI_STA
#define WIFI_MODE_AP    WIFI_AP
#define WIFI_MODE_APSTA WIFI_AP_STA

typedef enum {
    WIFI_AUTH_OPEN = 0,
    WIFI_AUTH_WEP,
    WIFI_AUTH_WPA_PSK,
    WIFI_AUTH_WPA2_PSK,
    WIFI_AUTH_WPA_WPA2_PSK,
    WIFI_AUTH_WPA2_ENTERPRISE,
    WIFI_AUTH_WPA3_PSK,
    WIFI_AUTH_WPA2_WPA3_PSK,
    WIFI_AUTH_WAPI_PSK,
    WIFI_AUTH_MAX
} wifi_auth_mode_t;

/* nombres de la libreria WiFi de Arduino */
#define ENC_TYPE_NONE WIFI_AUTH_OPEN
#define ENC_TYPE_WEP  WIFI_AUTH_WEP
#define ENC_TYPE_TKIP WIFI_AUTH_WPA_PSK
#define ENC_TYPE_CCMP WIFI_AUTH_WPA2_PSK
#define ENC_TYPE_AUTO WIFI_AUTH_WPA_WPA2_PSK

#define WIFI_SCAN_RUNNING (-1)
#define WIFI_SCAN_FAILED  (-2)

#endif

/*
 * WiFi.cpp - WiFi del W80x sobre el gestor de red (net manager) del WM IoT SDK.
 * Parte de w80x_arduino. LGPL 2.1.
 */
#include "WiFi.h"

extern "C" {
#include "wm_wifi.h"
#include "wm_wifi_event.h"
#include "wm_event.h"
#include "wm_nm_api.h"
#include "wm_netif.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/ip_addr.h"

/* el gestor de red se inicia desde su seccion de componentes: hay que
   pedirle al enlazador que lo incluya */
extern int __component_init_phase_wm_nm_core_init;
}

__attribute__((used)) static void *const _force_nm_init = &__component_init_phase_wm_nm_core_init;

WiFiClass WiFi;
static volatile bool s_scan_async = false; /* el resultado se recoge en el evento */

static void wifi_ev_cb(wm_event_group_t group, int event, void *data, void *priv)
{
    (void)group;
    ((WiFiClass *)priv)->_onWifiEvent(event, data);
}

static void nm_ev_cb(wm_event_group_t group, int event, void *data, void *priv)
{
    (void)group;
    ((WiFiClass *)priv)->_onNmEvent(event, data);
}

static wm_nm_netif_t sta_if() { return wm_nm_get_netif_by_name(WIFI_STATION_NETIF_NAME); }
static wm_nm_netif_t ap_if() { return wm_nm_get_netif_by_name(WIFI_SOFTAP_NETIF_NAME); }

static void to_wm(wm_ip_addr_t *w, IPAddress a)
{
    memset(w, 0, sizeof(*w));
    w->type             = WM_IPADDR_TYPE_V4;
    w->u_addr.ip4.addr  = (uint32_t)a;
}

WiFiClass::WiFiClass()
    : _mode(WIFI_OFF), _status(WL_IDLE_STATUS), _autoReconnect(true), _events(false), _staStarted(false),
      _apStarted(false), _static(false), _scanCount(WIFI_SCAN_FAILED), _scan(nullptr), _eventCb(nullptr)
{
    _ssid[0] = _pass[0] = _apSsid[0] = _apPass[0] = _hostname[0] = 0;
    memset(_bssid, 0, sizeof(_bssid));
}

bool WiFiClass::initEvents()
{
    if (!_events) {
        wm_wifi_init(); /* si ya estaba iniciado no pasa nada */
        wm_event_add_callback(WM_WIFI_EV, WM_EVENT_ANY_TYPE, wifi_ev_cb, this);
        wm_event_add_callback(WM_NM_EV, WM_EVENT_ANY_TYPE, nm_ev_cb, this);
        _events = true;
    }
    return true;
}

void WiFiClass::_onWifiEvent(int event, void *data)
{
    wm_wifi_event_data_t *d = (wm_wifi_event_data_t *)data;
    switch (event) {
        case WM_EVENT_WIFI_STA_CONNECTED:
            if (d) memcpy(_bssid, d->sta_connected_info.bssid, 6);
            if (_eventCb) _eventCb(ARDUINO_EVENT_WIFI_STA_CONNECTED);
            break;
        case WM_EVENT_WIFI_STA_DISCONNECTED:
            if (d) {
                int r = d->sta_disconnected_info.reason;
                if (r == WM_WIFI_REASON_NO_AP_FOUND) {
                    _status = WL_NO_SSID_AVAIL;
                } else if (r == WM_WIFI_REASON_AUTH_FAIL || r == WM_WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT ||
                           r == WM_WIFI_REASON_ASSOC_FAIL || r == WM_WIFI_REASON_AUTH_EXPIRE) {
                    _status = WL_CONNECT_FAILED;
                } else if (_status == WL_CONNECTED) {
                    _status = WL_CONNECTION_LOST;
                } else {
                    _status = WL_DISCONNECTED;
                }
            }
            if (!_autoReconnect && _staStarted) {
                _staStarted = false;
                wm_nm_stop_wifi_station();
            }
            if (_eventCb) _eventCb(ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
            break;
        case WM_EVENT_WIFI_AP_START:
            if (_eventCb) _eventCb(ARDUINO_EVENT_WIFI_AP_START);
            break;
        case WM_EVENT_WIFI_AP_STACONNECTED:
            if (_eventCb) _eventCb(ARDUINO_EVENT_WIFI_AP_STACONNECTED);
            break;
        case WM_EVENT_WIFI_AP_STADISCONNECTED:
            if (_eventCb) _eventCb(ARDUINO_EVENT_WIFI_AP_STADISCONNECTED);
            break;
        case WM_EVENT_WIFI_SCAN_DONE:
            if (_scanCount == WIFI_SCAN_RUNNING && (s_scan_async || data == NULL)) {
                s_scan_async = false;
                uint16_t n = 0;
                wm_wifi_scan_get_ap_num(&n);
                free(_scan);
                _scan = n ? calloc(n, sizeof(wm_wifi_ap_record_t)) : nullptr;
                if (_scan) {
                    wm_wifi_scan_get_ap_records(&n, (wm_wifi_ap_record_t *)_scan);
                } else {
                    n = 0;
                }
                _scanCount = n;
            }
            if (_eventCb) _eventCb(ARDUINO_EVENT_WIFI_SCAN_DONE);
            break;
        default:
            break;
    }
}

void WiFiClass::_onNmEvent(int event, void *data)
{
    (void)data;
    switch (event) {
        case WM_NM_WIFI_STA_CONNECTED:
            if (_static) {
                applyStaticIP();
                _status = WL_CONNECTED;
            }
            break;
        case WM_NM_WIFI_STA_GOT_IP:
            _status = WL_CONNECTED;
            if (_hostname[0]) {
                wm_netif_set_hostname(WM_NETIF_TYPE_WIFI_STA, _hostname);
            }
            if (_eventCb) _eventCb(ARDUINO_EVENT_WIFI_STA_GOT_IP);
            break;
        case WM_NM_WIFI_STA_LOST_IP:
            if (_status == WL_CONNECTED) {
                _status = WL_CONNECTION_LOST;
            }
            if (_eventCb) _eventCb(ARDUINO_EVENT_WIFI_STA_LOST_IP);
            break;
        default:
            break;
    }
}

void WiFiClass::applyStaticIP()
{
    wm_nm_netif_t nif = sta_if();
    if (nif == WM_NM_INVALID_NETIF) {
        return;
    }
    wm_nm_stop_netif_dhcpc(nif);
    wm_netif_ip_info_t info;
    to_wm(&info.ip, _ip);
    to_wm(&info.netmask, _mask);
    to_wm(&info.gw, _gw);
    wm_nm_set_netif_ip_info(nif, &info);
    wm_ip_addr_t d;
    if ((uint32_t)_dns1) {
        to_wm(&d, _dns1);
        wm_nm_set_dns(WM_NM_DNS_MAIN, &d);
    }
    if ((uint32_t)_dns2) {
        to_wm(&d, _dns2);
        wm_nm_set_dns(WM_NM_DNS_BACKUP, &d);
    }
}

/* ------------------------------------------------------------------ estacion */

wl_status_t WiFiClass::begin(const char *ssid, const char *passphrase, int32_t channel, const uint8_t *bssid, bool connect)
{
    (void)channel;
    (void)bssid;
    if (!ssid || !*ssid || strlen(ssid) > 32 || (passphrase && strlen(passphrase) > 64)) {
        _status = WL_CONNECT_FAILED;
        return _status;
    }
    strncpy(_ssid, ssid, sizeof(_ssid) - 1);
    _ssid[sizeof(_ssid) - 1] = 0;
    strncpy(_pass, passphrase ? passphrase : "", sizeof(_pass) - 1);
    _pass[sizeof(_pass) - 1] = 0;
    if (!connect) {
        return _status;
    }
    return begin();
}

wl_status_t WiFiClass::begin()
{
    if (!_ssid[0]) {
        return WL_CONNECT_FAILED;
    }
    initEvents();
    if (_staStarted) {
        wm_nm_stop_wifi_station();
        delay(100);
    }
    wm_nm_wifi_station_cfg_t cfg = { _ssid, _pass[0] ? _pass : NULL };
    _status = WL_DISCONNECTED;
    if (wm_nm_start_wifi_station(&cfg) != WM_ERR_SUCCESS) {
        _status = WL_CONNECT_FAILED;
        return _status;
    }
    _staStarted = true;
    _mode       = (wifi_mode_t)(_mode | WIFI_STA);
    return _status;
}

bool WiFiClass::config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1, IPAddress dns2)
{
    if ((uint32_t)local_ip == 0) {
        /* 0.0.0.0: volver a DHCP */
        _static = false;
        if (_staStarted && sta_if() != WM_NM_INVALID_NETIF) {
            wm_nm_start_netif_dhcpc(sta_if());
        }
        return true;
    }
    _ip     = local_ip;
    _gw     = gateway;
    _mask   = subnet;
    _dns1   = (uint32_t)dns1 ? dns1 : gateway;
    _dns2   = dns2;
    _static = true;
    if (status() == WL_CONNECTED) {
        applyStaticIP();
    }
    return true;
}

bool WiFiClass::disconnect(bool wifioff, bool eraseap)
{
    if (_staStarted) {
        wm_nm_stop_wifi_station();
        _staStarted = false;
    }
    _status = WL_DISCONNECTED;
    if (eraseap) {
        _ssid[0] = _pass[0] = 0;
    }
    _mode = (wifi_mode_t)(_mode & ~WIFI_STA);
    if (wifioff) {
        mode(WIFI_OFF);
    }
    return true;
}

bool WiFiClass::reconnect()
{
    return begin() != WL_CONNECT_FAILED;
}

wl_status_t WiFiClass::status()
{
    if (!_staStarted) {
        return _status == WL_CONNECTED ? WL_DISCONNECTED : _status;
    }
    wm_nm_state_e s = wm_nm_get_wifi_station_state();
    if (s == WM_NM_WIFI_STA_GOT_IP || (_static && s == WM_NM_WIFI_STA_CONNECTED)) {
        _status = WL_CONNECTED;
    } else if (_status == WL_CONNECTED) {
        _status = WL_CONNECTION_LOST;
    }
    return _status;
}

uint8_t WiFiClass::waitForConnectResult(unsigned long timeoutLength)
{
    unsigned long t0 = millis();
    while (millis() - t0 < timeoutLength) {
        wl_status_t s = status();
        if (s == WL_CONNECTED || s == WL_CONNECT_FAILED || s == WL_NO_SSID_AVAIL) {
            return s;
        }
        delay(100);
    }
    return status();
}

bool WiFiClass::setHostname(const char *hostname)
{
    if (!hostname || strlen(hostname) > 32) {
        return false;
    }
    strcpy(_hostname, hostname);
    if (status() == WL_CONNECTED) {
        wm_netif_set_hostname(WM_NETIF_TYPE_WIFI_STA, _hostname);
    }
    return true;
}

static bool ip_info(wm_nm_netif_t nif, wm_netif_ip_info_t *info)
{
    memset(info, 0, sizeof(*info));
    return nif != WM_NM_INVALID_NETIF && wm_nm_get_netif_ip_info(nif, info) == WM_ERR_SUCCESS;
}

IPAddress WiFiClass::localIP()
{
    wm_netif_ip_info_t i;
    return ip_info(sta_if(), &i) ? IPAddress(i.ip.u_addr.ip4.addr) : IPAddress();
}

IPAddress WiFiClass::subnetMask()
{
    wm_netif_ip_info_t i;
    return ip_info(sta_if(), &i) ? IPAddress(i.netmask.u_addr.ip4.addr) : IPAddress();
}

IPAddress WiFiClass::gatewayIP()
{
    wm_netif_ip_info_t i;
    return ip_info(sta_if(), &i) ? IPAddress(i.gw.u_addr.ip4.addr) : IPAddress();
}

IPAddress WiFiClass::dnsIP(uint8_t dns_no)
{
    wm_ip_addr_t d;
    memset(&d, 0, sizeof(d));
    if (wm_nm_get_dns(dns_no ? WM_NM_DNS_BACKUP : WM_NM_DNS_MAIN, &d) != WM_ERR_SUCCESS) {
        return IPAddress();
    }
    return IPAddress(d.u_addr.ip4.addr);
}

IPAddress WiFiClass::broadcastIP()
{
    wm_netif_ip_info_t i;
    if (!ip_info(sta_if(), &i)) {
        return IPAddress();
    }
    return IPAddress((uint32_t)(i.ip.u_addr.ip4.addr | ~i.netmask.u_addr.ip4.addr));
}

static String mac_str(const uint8_t *m)
{
    char s[18];
    snprintf(s, sizeof(s), "%02X:%02X:%02X:%02X:%02X:%02X", m[0], m[1], m[2], m[3], m[4], m[5]);
    return String(s);
}

uint8_t *WiFiClass::macAddress(uint8_t *mac)
{
    initEvents();
    memset(mac, 0, 6);
    wm_wifi_get_mac(WM_WIFI_IF_STA, mac);
    return mac;
}

String WiFiClass::macAddress()
{
    uint8_t m[6];
    return mac_str(macAddress(m));
}

String WiFiClass::SSID()
{
    wm_wifi_ap_record_t r;
    if (status() == WL_CONNECTED && wm_wifi_sta_get_ap_info(&r) == WM_ERR_SUCCESS) {
        return String((const char *)r.ssid);
    }
    return String(_ssid);
}

uint8_t *WiFiClass::BSSID()
{
    wm_wifi_ap_record_t r;
    if (wm_wifi_sta_get_ap_info(&r) == WM_ERR_SUCCESS) {
        memcpy(_bssid, r.bssid, 6);
    }
    return _bssid;
}

String WiFiClass::BSSIDstr()
{
    return mac_str(BSSID());
}

int8_t WiFiClass::RSSI()
{
    wm_wifi_ap_record_t r;
    if (status() == WL_CONNECTED && wm_wifi_sta_get_ap_info(&r) == WM_ERR_SUCCESS) {
        return r.rssi;
    }
    return 0;
}

int32_t WiFiClass::channel()
{
    wm_wifi_ap_record_t r;
    if (wm_wifi_sta_get_ap_info(&r) == WM_ERR_SUCCESS) {
        return r.primary;
    }
    return 0;
}

/* ------------------------------------------------------------------ punto de acceso */

bool WiFiClass::softAP(const char *ssid, const char *passphrase, int channel, int ssid_hidden, int max_connection)
{
    (void)channel; /* el SDK elige el canal (el de la estacion, o el 1) */
    (void)ssid_hidden;
    (void)max_connection;
    if (!ssid || !*ssid || strlen(ssid) > 32) {
        return false;
    }
    if (passphrase && *passphrase && (strlen(passphrase) < 8 || strlen(passphrase) > 64)) {
        return false; /* WPA2 pide de 8 a 63 caracteres */
    }
    initEvents();
    strcpy(_apSsid, ssid);
    strcpy(_apPass, passphrase ? passphrase : "");
    if (_apStarted) {
        wm_nm_stop_wifi_softap();
        delay(100);
    }
    wm_nm_wifi_softap_cfg_t cfg = { _apSsid, _apPass[0] ? _apPass : NULL };
    if (wm_nm_start_wifi_softap(&cfg) != WM_ERR_SUCCESS) {
        return false;
    }
    _apStarted = true;
    _mode      = (wifi_mode_t)(_mode | WIFI_AP);
    uint32_t t0 = millis();
    while (wm_nm_get_wifi_softap_state() != WM_NM_WIFI_AP_READY && millis() - t0 < 5000) {
        delay(20);
    }
    return wm_nm_get_wifi_softap_state() == WM_NM_WIFI_AP_READY ||
           wm_nm_get_wifi_softap_state() == WM_NM_WIFI_AP_ASSIGNED_IP_ID;
}

bool WiFiClass::softAPdisconnect(bool wifioff)
{
    if (_apStarted) {
        wm_nm_stop_wifi_softap();
        _apStarted = false;
    }
    _mode = (wifi_mode_t)(_mode & ~WIFI_AP);
    if (wifioff) {
        mode(WIFI_OFF);
    }
    return true;
}

IPAddress WiFiClass::softAPIP()
{
    wm_netif_ip_info_t i;
    return ip_info(ap_if(), &i) ? IPAddress(i.ip.u_addr.ip4.addr) : IPAddress();
}

IPAddress WiFiClass::softAPSubnetMask()
{
    wm_netif_ip_info_t i;
    return ip_info(ap_if(), &i) ? IPAddress(i.netmask.u_addr.ip4.addr) : IPAddress();
}

uint8_t WiFiClass::softAPgetStationNum()
{
    wm_wifi_sta_list_t l;
    memset(&l, 0, sizeof(l));
    if (!_apStarted || wm_wifi_ap_get_sta_list(&l) != WM_ERR_SUCCESS) {
        return 0;
    }
    return l.num;
}

uint8_t *WiFiClass::softAPmacAddress(uint8_t *mac)
{
    initEvents();
    memset(mac, 0, 6);
    wm_wifi_get_mac(WM_WIFI_IF_AP, mac);
    return mac;
}

String WiFiClass::softAPmacAddress()
{
    uint8_t m[6];
    return mac_str(softAPmacAddress(m));
}

/* ------------------------------------------------------------------ modo */

bool WiFiClass::mode(wifi_mode_t m)
{
    if (!(m & WIFI_STA) && _staStarted) {
        disconnect();
    }
    if (!(m & WIFI_AP) && _apStarted) {
        softAPdisconnect();
    }
    if (m != WIFI_OFF) {
        initEvents();
    }
    _mode = m;
    return true;
}

bool WiFiClass::enableSTA(bool enable)
{
    return mode((wifi_mode_t)(enable ? (_mode | WIFI_STA) : (_mode & ~WIFI_STA)));
}

bool WiFiClass::enableAP(bool enable)
{
    return mode((wifi_mode_t)(enable ? (_mode | WIFI_AP) : (_mode & ~WIFI_AP)));
}

/* ------------------------------------------------------------------ busqueda */

int16_t WiFiClass::scanNetworks(bool async, bool show_hidden)
{
    initEvents();
    scanDelete();
    wm_wifi_scan_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.show_hidden = show_hidden;
    _scanCount      = WIFI_SCAN_RUNNING;
    s_scan_async    = async;
    if (wm_wifi_scan_start(&cfg, !async) != WM_ERR_SUCCESS) {
        s_scan_async = false;
        _scanCount = WIFI_SCAN_FAILED;
        return _scanCount;
    }
    if (async) {
        return WIFI_SCAN_RUNNING;
    }
    /* bloqueante: el resultado se recoge aqui */
    _onWifiEvent(WM_EVENT_WIFI_SCAN_DONE, NULL);
    return _scanCount;
}

void WiFiClass::scanDelete()
{
    free(_scan);
    _scan      = nullptr;
    _scanCount = WIFI_SCAN_FAILED;
}

static wm_wifi_ap_record_t *rec(void *scan, int16_t count, uint8_t i)
{
    if (!scan || i >= count) {
        return nullptr;
    }
    return &((wm_wifi_ap_record_t *)scan)[i];
}

String WiFiClass::SSID(uint8_t i)
{
    wm_wifi_ap_record_t *r = rec(_scan, _scanCount, i);
    return r ? String((const char *)r->ssid) : String();
}

int32_t WiFiClass::RSSI(uint8_t i)
{
    wm_wifi_ap_record_t *r = rec(_scan, _scanCount, i);
    return r ? r->rssi : 0;
}

uint8_t WiFiClass::encryptionType(uint8_t i)
{
    wm_wifi_ap_record_t *r = rec(_scan, _scanCount, i);
    return r ? (uint8_t)r->authmode : WIFI_AUTH_OPEN;
}

int32_t WiFiClass::channel(uint8_t i)
{
    wm_wifi_ap_record_t *r = rec(_scan, _scanCount, i);
    return r ? r->primary : 0;
}

uint8_t *WiFiClass::BSSID(uint8_t i)
{
    wm_wifi_ap_record_t *r = rec(_scan, _scanCount, i);
    return r ? r->bssid : nullptr;
}

String WiFiClass::BSSIDstr(uint8_t i)
{
    uint8_t *b = BSSID(i);
    return b ? mac_str(b) : String();
}

/* ------------------------------------------------------------------ DNS */

int WiFiClass::hostByName(const char *host, IPAddress &result)
{
    if (result.fromString(host)) {
        return 1;
    }
    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (::lwip_getaddrinfo(host, NULL, &hints, &res) != 0 || res == NULL) {
        return 0;
    }
    result = IPAddress((uint32_t)((struct sockaddr_in *)res->ai_addr)->sin_addr.s_addr);
    ::lwip_freeaddrinfo(res);
    return 1;
}

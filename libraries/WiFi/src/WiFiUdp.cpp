/*
 * WiFiUdp.cpp - UDP sobre sockets de lwIP.
 * Parte de w80x_arduino. LGPL 2.1.
 */
#include "WiFi.h"
#include "WiFiUdp.h"

extern "C" {
#include "lwip/sockets.h"
#include "lwip/errno.h"
}

WiFiUDP::WiFiUDP()
    : _fd(-1), _port(0), _remotePort(0), _tx(nullptr), _txLen(0), _rx(nullptr), _rxLen(0), _rxIdx(0)
{
}

WiFiUDP::~WiFiUDP()
{
    stop();
}

bool WiFiUDP::open()
{
    if (_fd >= 0) {
        return true;
    }
    _fd = ::lwip_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (_fd < 0) {
        return false;
    }
    int one = 1;
    ::lwip_setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    ::lwip_setsockopt(_fd, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one));
    ::lwip_fcntl(_fd, F_SETFL, ::lwip_fcntl(_fd, F_GETFL, 0) | O_NONBLOCK);
    return true;
}

uint8_t WiFiUDP::begin(IPAddress address, uint16_t port)
{
    stop();
    if (!open()) {
        return 0;
    }
    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family      = AF_INET;
    a.sin_port        = htons(port);
    a.sin_addr.s_addr = (uint32_t)address;
    if (::lwip_bind(_fd, (struct sockaddr *)&a, sizeof(a)) < 0) {
        stop();
        return 0;
    }
    _port = port;
    return 1;
}

uint8_t WiFiUDP::begin(uint16_t port)
{
    return begin(IPAddress((uint32_t)INADDR_ANY), port);
}

uint8_t WiFiUDP::beginMulticast(IPAddress multicast, uint16_t port)
{
    if (!begin(port)) {
        return 0;
    }
    struct ip_mreq m;
    m.imr_multiaddr.s_addr = (uint32_t)multicast;
    m.imr_interface.s_addr = INADDR_ANY;
    if (::lwip_setsockopt(_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &m, sizeof(m)) < 0) {
        stop();
        return 0;
    }
    _multicast = multicast;
    return 1;
}

void WiFiUDP::stop()
{
    if (_fd >= 0) {
        ::lwip_close(_fd);
        _fd = -1;
    }
    free(_tx);
    free(_rx);
    _tx    = nullptr;
    _rx    = nullptr;
    _txLen = _rxLen = _rxIdx = 0;
}

int WiFiUDP::beginPacket(IPAddress ip, uint16_t port)
{
    if (!open()) {
        return 0;
    }
    if (!_tx) {
        _tx = (uint8_t *)malloc(WIFI_UDP_BUFFER_SIZE);
        if (!_tx) return 0;
    }
    _remoteIP   = ip;
    _remotePort = port;
    _txLen      = 0;
    return 1;
}

int WiFiUDP::beginPacket(const char *host, uint16_t port)
{
    IPAddress ip;
    if (!WiFi.hostByName(host, ip)) {
        return 0;
    }
    return beginPacket(ip, port);
}

size_t WiFiUDP::write(uint8_t b)
{
    if (!_tx || _txLen >= WIFI_UDP_BUFFER_SIZE) {
        return 0;
    }
    _tx[_txLen++] = b;
    return 1;
}

size_t WiFiUDP::write(const uint8_t *buffer, size_t size)
{
    size_t n = 0;
    while (n < size && write(buffer[n])) {
        n++;
    }
    return n;
}

int WiFiUDP::endPacket()
{
    if (_fd < 0 || !_tx) {
        return 0;
    }
    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family      = AF_INET;
    a.sin_port        = htons(_remotePort);
    a.sin_addr.s_addr = (uint32_t)_remoteIP;
    int r  = ::lwip_sendto(_fd, _tx, _txLen, 0, (struct sockaddr *)&a, sizeof(a));
    _txLen = 0;
    return r >= 0 ? 1 : 0;
}

int WiFiUDP::parsePacket()
{
    if (_fd < 0) {
        return 0;
    }
    if (!_rx) {
        _rx = (uint8_t *)malloc(WIFI_UDP_BUFFER_SIZE);
        if (!_rx) return 0;
    }
    struct sockaddr_in a;
    socklen_t l = sizeof(a);
    int r       = ::lwip_recvfrom(_fd, _rx, WIFI_UDP_BUFFER_SIZE, MSG_DONTWAIT, (struct sockaddr *)&a, &l);
    if (r <= 0) {
        _rxLen = _rxIdx = 0;
        return 0;
    }
    _remoteIP   = IPAddress((uint32_t)a.sin_addr.s_addr);
    _remotePort = ntohs(a.sin_port);
    _rxLen      = r;
    _rxIdx      = 0;
    return r;
}

int WiFiUDP::available()
{
    return (int)(_rxLen - _rxIdx);
}

int WiFiUDP::read()
{
    return _rxIdx < _rxLen ? _rx[_rxIdx++] : -1;
}

int WiFiUDP::read(unsigned char *buffer, size_t len)
{
    size_t n = _rxLen - _rxIdx;
    if (n > len) n = len;
    if (n == 0) return 0;
    memcpy(buffer, _rx + _rxIdx, n);
    _rxIdx += n;
    return n;
}

int WiFiUDP::peek()
{
    return _rxIdx < _rxLen ? _rx[_rxIdx] : -1;
}

void WiFiUDP::flush()
{
    _rxLen = _rxIdx = 0;
}

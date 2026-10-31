/*
 * WiFiUdp.h - UDP sobre lwIP, como WiFiUDP de Arduino/ESP32.
 * Parte de w80x_arduino. LGPL 2.1.
 */
#ifndef WiFiUdp_h
#define WiFiUdp_h

#include <Arduino.h>
#include "Udp.h"

#ifndef WIFI_UDP_BUFFER_SIZE
#define WIFI_UDP_BUFFER_SIZE 1460
#endif

class WiFiUDP : public UDP
{
public:
    WiFiUDP();
    ~WiFiUDP();

    uint8_t begin(uint16_t port) override;
    uint8_t begin(IPAddress address, uint16_t port);
    uint8_t beginMulticast(IPAddress multicast, uint16_t port) override;
    void stop() override;

    int beginPacket() { return beginPacket(_remoteIP, _remotePort); }
    int beginPacket(IPAddress ip, uint16_t port) override;
    int beginPacket(const char *host, uint16_t port) override;
    int beginMulticastPacket() { return beginPacket(_multicast, _port); }
    int endPacket() override;
    size_t write(uint8_t) override;
    size_t write(const uint8_t *buffer, size_t size) override;
    using Print::write;

    int parsePacket() override;
    int available() override;
    int read() override;
    int read(unsigned char *buffer, size_t len) override;
    int read(char *buffer, size_t len) override { return read((unsigned char *)buffer, len); }
    int peek() override;
    void flush() override;
    IPAddress remoteIP() override { return _remoteIP; }
    uint16_t remotePort() override { return _remotePort; }

private:
    bool open();
    int _fd;
    uint16_t _port;
    IPAddress _multicast;
    IPAddress _remoteIP;
    uint16_t _remotePort;
    uint8_t *_tx;
    size_t _txLen;
    uint8_t *_rx;
    size_t _rxLen, _rxIdx;
};

#endif

/*
 * WiFiClient.h - cliente TCP (sockets de lwIP), como WiFiClient del ESP32.
 * Las copias del objeto comparten la misma conexion.
 * Parte de w80x_arduino. LGPL 2.1.
 */
#ifndef WiFiClient_h
#define WiFiClient_h

#include <Arduino.h>
#include "Client.h"

class WiFiClientSocket; /* conexion compartida entre copias */

class WiFiClient : public Client
{
public:
    WiFiClient();
    explicit WiFiClient(int fd);
    WiFiClient(const WiFiClient &other);
    WiFiClient &operator=(const WiFiClient &other);
    ~WiFiClient();

    int connect(IPAddress ip, uint16_t port) override { return connect(ip, port, _timeout_ms); }
    int connect(IPAddress ip, uint16_t port, int32_t timeout_ms);
    int connect(const char *host, uint16_t port) override { return connect(host, port, _timeout_ms); }
    int connect(const char *host, uint16_t port, int32_t timeout_ms);

    size_t write(uint8_t data) override { return write(&data, 1); }
    size_t write(const uint8_t *buf, size_t size) override;
    size_t write_P(PGM_P buf, size_t size) { return write((const uint8_t *)buf, size); }
    size_t write(Stream &stream);
    using Print::write;

    int available() override;
    int read() override;
    int read(uint8_t *buf, size_t size) override;
    int read(char *buf, size_t size) { return read((uint8_t *)buf, size); }
    int peek() override;
    void flush() override {}
    void stop() override;
    uint8_t connected() override;
    operator bool() override { return connected(); }
    bool operator==(const WiFiClient &rhs) const { return _sock == rhs._sock; }
    bool operator!=(const WiFiClient &rhs) const { return _sock != rhs._sock; }

    int fd() const;
    int setNoDelay(bool nodelay);
    bool getNoDelay();
    void setConnectionTimeout(uint32_t ms) { _timeout_ms = ms; }
    int setTimeout(uint32_t seconds); /* lectura/escritura bloqueantes */

    IPAddress remoteIP() const;
    uint16_t remotePort() const;
    IPAddress localIP() const;
    uint16_t localPort() const;

private:
    void release();
    WiFiClientSocket *_sock;
    int32_t _timeout_ms;
};

#endif

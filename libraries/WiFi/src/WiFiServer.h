/*
 * WiFiServer.h - servidor TCP, como WiFiServer del ESP32.
 *   WiFiServer server(80);  server.begin();
 *   WiFiClient c = server.available();   // o accept()
 * Parte de w80x_arduino. LGPL 2.1.
 */
#ifndef WiFiServer_h
#define WiFiServer_h

#include <Arduino.h>
#include "Server.h"
#include "WiFiClient.h"

class WiFiServer : public Server
{
public:
    explicit WiFiServer(uint16_t port = 80, uint8_t max_clients = 4) : _fd(-1), _port(port), _max(max_clients), _noDelay(true) {}
    ~WiFiServer() { end(); }

    void begin() override { begin(_port); }
    void begin(uint16_t port, int reuse_enable = 1);
    void end();
    void close() { end(); }
    void stop() { end(); }
    WiFiClient accept();
    WiFiClient available() { return accept(); }
    bool hasClient();
    /* por defecto sin retardo de Nagle: las respuestas cortas salen al momento */
    void setNoDelay(bool nodelay) { _noDelay = nodelay; }
    bool getNoDelay() { return _noDelay; }
    operator bool() { return _fd >= 0; }

    /* escribir a todos los clientes no esta disponible (como en ESP32) */
    size_t write(uint8_t) override { return 0; }
    size_t write(const uint8_t *, size_t) override { return 0; }
    using Print::write;

private:
    int _fd;
    uint16_t _port;
    uint8_t _max;
    bool _noDelay;
    int _pending = -1; /* conexion aceptada por hasClient() */
};

#endif

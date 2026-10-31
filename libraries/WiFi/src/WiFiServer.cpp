/*
 * WiFiServer.cpp - servidor TCP sobre sockets de lwIP.
 * Parte de w80x_arduino. LGPL 2.1.
 */
#include "WiFiServer.h"

extern "C" {
#include "lwip/sockets.h"
#include "lwip/errno.h"
}

void WiFiServer::begin(uint16_t port, int reuse_enable)
{
    if (_fd >= 0) {
        return;
    }
    if (port) {
        _port = port;
    }
    int fd = ::lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) {
        return;
    }
    if (reuse_enable) {
        int one = 1;
        ::lwip_setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    }
    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family      = AF_INET;
    a.sin_port        = htons(_port);
    a.sin_addr.s_addr = INADDR_ANY;
    if (::lwip_bind(fd, (struct sockaddr *)&a, sizeof(a)) < 0 || ::lwip_listen(fd, _max) < 0) {
        ::lwip_close(fd);
        return;
    }
    ::lwip_fcntl(fd, F_SETFL, ::lwip_fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
    _fd = fd;
}

void WiFiServer::end()
{
    if (_pending >= 0) {
        ::lwip_close(_pending);
        _pending = -1;
    }
    if (_fd >= 0) {
        ::lwip_close(_fd);
        _fd = -1;
    }
}

bool WiFiServer::hasClient()
{
    if (_pending >= 0) {
        return true;
    }
    if (_fd < 0) {
        return false;
    }
    struct sockaddr_in a;
    socklen_t l = sizeof(a);
    int c = ::lwip_accept(_fd, (struct sockaddr *)&a, &l);
    if (c >= 0) {
        _pending = c;
        return true;
    }
    return false;
}

WiFiClient WiFiServer::accept()
{
    if (!hasClient()) {
        return WiFiClient();
    }
    int c    = _pending;
    _pending = -1;
    /* el socket del cliente es bloqueante (con tiempo maximo) */
    ::lwip_fcntl(c, F_SETFL, ::lwip_fcntl(c, F_GETFL, 0) & ~O_NONBLOCK);
    struct timeval tv = { 3, 0 };
    ::lwip_setsockopt(c, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    ::lwip_setsockopt(c, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    int one = 1;
    ::lwip_setsockopt(c, SOL_SOCKET, SO_KEEPALIVE, &one, sizeof(one));
    if (_noDelay) {
        ::lwip_setsockopt(c, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
    }
    return WiFiClient(c);
}

/*
 * WiFiClient.cpp - cliente TCP sobre sockets de lwIP.
 * Parte de w80x_arduino. LGPL 2.1.
 */
#include "WiFi.h"
#include "WiFiClient.h"

extern "C" {
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/errno.h"
}

class WiFiClientSocket
{
public:
    explicit WiFiClientSocket(int f) : fd(f), refs(1), peeked(-1), closed(false) {}
    ~WiFiClientSocket()
    {
        if (fd >= 0) {
            ::lwip_close(fd);
        }
    }
    int fd;
    int refs;
    int peeked; /* byte leido con peek() aun no entregado */
    bool closed;
};

WiFiClient::WiFiClient() : _sock(nullptr), _timeout_ms(3000) {}

WiFiClient::WiFiClient(int fd) : _sock(nullptr), _timeout_ms(3000)
{
    if (fd >= 0) {
        _sock = new WiFiClientSocket(fd);
    }
}

WiFiClient::WiFiClient(const WiFiClient &other) : _sock(other._sock), _timeout_ms(other._timeout_ms)
{
    if (_sock) {
        _sock->refs++;
    }
}

WiFiClient &WiFiClient::operator=(const WiFiClient &other)
{
    if (this != &other) {
        if (other._sock) {
            other._sock->refs++;
        }
        release();
        _sock       = other._sock;
        _timeout_ms = other._timeout_ms;
    }
    return *this;
}

WiFiClient::~WiFiClient()
{
    release();
}

void WiFiClient::release()
{
    if (_sock && --_sock->refs == 0) {
        delete _sock;
    }
    _sock = nullptr;
}

int WiFiClient::fd() const
{
    return _sock ? _sock->fd : -1;
}

int WiFiClient::connect(IPAddress ip, uint16_t port, int32_t timeout_ms)
{
    stop();
    int fd = ::lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) {
        return 0;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = (uint32_t)ip;

    int flags = ::lwip_fcntl(fd, F_GETFL, 0);
    ::lwip_fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    int r = ::lwip_connect(fd, (struct sockaddr *)&addr, sizeof(addr));
    if (r < 0 && errno != EINPROGRESS) {
        ::lwip_close(fd);
        return 0;
    }
    if (r < 0) {
        fd_set wset;
        FD_ZERO(&wset);
        FD_SET(fd, &wset);
        struct timeval tv;
        tv.tv_sec  = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        r = ::lwip_select(fd + 1, NULL, &wset, NULL, timeout_ms < 0 ? NULL : &tv);
        int err = 0;
        socklen_t len = sizeof(err);
        if (r <= 0 || ::lwip_getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len) < 0 || err != 0) {
            ::lwip_close(fd);
            return 0;
        }
    }
    ::lwip_fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    /* lecturas y escrituras bloqueantes con tiempo maximo */
    struct timeval tv;
    tv.tv_sec  = _timeout / 1000;
    tv.tv_usec = (_timeout % 1000) * 1000;
    ::lwip_setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    ::lwip_setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    int one = 1;
    ::lwip_setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
    ::lwip_setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &one, sizeof(one));
    _sock = new WiFiClientSocket(fd);
    return 1;
}

int WiFiClient::connect(const char *host, uint16_t port, int32_t timeout_ms)
{
    IPAddress ip;
    if (!WiFi.hostByName(host, ip)) {
        return 0;
    }
    return connect(ip, port, timeout_ms);
}

int WiFiClient::setTimeout(uint32_t seconds)
{
    Stream::setTimeout(seconds * 1000);
    if (!_sock) {
        return 0;
    }
    struct timeval tv;
    tv.tv_sec  = seconds;
    tv.tv_usec = 0;
    ::lwip_setsockopt(_sock->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    return ::lwip_setsockopt(_sock->fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

int WiFiClient::setNoDelay(bool nodelay)
{
    if (!_sock) return 0;
    int v = nodelay;
    return ::lwip_setsockopt(_sock->fd, IPPROTO_TCP, TCP_NODELAY, &v, sizeof(v));
}

bool WiFiClient::getNoDelay()
{
    if (!_sock) return false;
    int v = 0;
    socklen_t l = sizeof(v);
    ::lwip_getsockopt(_sock->fd, IPPROTO_TCP, TCP_NODELAY, &v, &l);
    return v != 0;
}

size_t WiFiClient::write(const uint8_t *buf, size_t size)
{
    if (!_sock || _sock->closed || size == 0) {
        return 0;
    }
    size_t sent    = 0;
    int retries    = 10;
    while (sent < size) {
        int r = ::lwip_send(_sock->fd, buf + sent, size - sent, 0);
        if (r > 0) {
            sent += r;
            retries = 10;
        } else if (r < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == ENOMEM) && retries-- > 0) {
            delay(10);
        } else {
            _sock->closed = true;
            break;
        }
    }
    return sent;
}

size_t WiFiClient::write(Stream &stream)
{
    uint8_t buf[128];
    size_t total = 0;
    while (stream.available()) {
        size_t n = stream.readBytes(buf, sizeof(buf));
        if (n == 0) break;
        size_t w = write(buf, n);
        total += w;
        if (w != n) break;
    }
    return total;
}

int WiFiClient::available()
{
    if (!_sock) {
        return 0;
    }
    int n = 0;
    if (::lwip_ioctl(_sock->fd, FIONREAD, &n) < 0) {
        n = 0;
    }
    return n + (_sock->peeked >= 0 ? 1 : 0);
}

int WiFiClient::read(uint8_t *buf, size_t size)
{
    if (!_sock || size == 0) {
        return -1;
    }
    size_t got = 0;
    if (_sock->peeked >= 0) {
        buf[got++]    = (uint8_t)_sock->peeked;
        _sock->peeked = -1;
    }
    if (got < size) {
        int r = ::lwip_recv(_sock->fd, buf + got, size - got, MSG_DONTWAIT);
        if (r > 0) {
            got += r;
        } else if (r == 0) {
            _sock->closed = true;
        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
            _sock->closed = true;
        }
    }
    return got > 0 ? (int)got : -1;
}

int WiFiClient::read()
{
    uint8_t b;
    return read(&b, 1) == 1 ? b : -1;
}

int WiFiClient::peek()
{
    if (!_sock) {
        return -1;
    }
    if (_sock->peeked < 0) {
        uint8_t b;
        int r = ::lwip_recv(_sock->fd, &b, 1, MSG_DONTWAIT);
        if (r == 1) {
            _sock->peeked = b;
        } else if (r == 0) {
            _sock->closed = true;
        }
    }
    return _sock->peeked;
}

void WiFiClient::stop()
{
    if (_sock) {
        /* se cierra para todas las copias */
        if (_sock->fd >= 0) {
            ::lwip_close(_sock->fd);
            _sock->fd = -1;
        }
        _sock->closed = true;
    }
    release();
}

uint8_t WiFiClient::connected()
{
    if (!_sock || _sock->fd < 0) {
        return 0;
    }
    if (available() > 0) {
        return 1; /* aun quedan datos por leer */
    }
    if (_sock->closed) {
        return 0;
    }
    uint8_t b;
    int r = ::lwip_recv(_sock->fd, &b, 1, MSG_DONTWAIT | MSG_PEEK);
    if (r == 0) {
        _sock->closed = true;
        return 0;
    }
    if (r < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        _sock->closed = true;
        return 0;
    }
    return 1;
}

static bool sock_addr(int fd, bool peer, struct sockaddr_in *a)
{
    socklen_t l = sizeof(*a);
    if (fd < 0) return false;
    int r = peer ? ::lwip_getpeername(fd, (struct sockaddr *)a, &l) : ::lwip_getsockname(fd, (struct sockaddr *)a, &l);
    return r == 0;
}

IPAddress WiFiClient::remoteIP() const
{
    struct sockaddr_in a;
    return sock_addr(fd(), true, &a) ? IPAddress((uint32_t)a.sin_addr.s_addr) : IPAddress();
}

uint16_t WiFiClient::remotePort() const
{
    struct sockaddr_in a;
    return sock_addr(fd(), true, &a) ? ntohs(a.sin_port) : 0;
}

IPAddress WiFiClient::localIP() const
{
    struct sockaddr_in a;
    return sock_addr(fd(), false, &a) ? IPAddress((uint32_t)a.sin_addr.s_addr) : IPAddress();
}

uint16_t WiFiClient::localPort() const
{
    struct sockaddr_in a;
    return sock_addr(fd(), false, &a) ? ntohs(a.sin_port) : 0;
}

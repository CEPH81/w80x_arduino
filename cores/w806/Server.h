/*
 * Server.h - clase base de los servidores de red (Arduino). LGPL 2.1.
 */
#ifndef server_h
#define server_h
#include "Print.h"

class Server : public Print
{
public:
    virtual void begin() = 0;
};

#endif

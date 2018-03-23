/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_NET_H__
#define __MOBICAST_NET_H__

#include <string>
#include <list>

namespace MobiCast
{

/** NetworkInterface struct. Network interface info structure. */
struct NetworkInterface
{
    std::string adapter;    /** Adapter name of the network interface. */
    std::string ipv4;       /** IPv4 address of the network interface. */
};

/** Returns a list of network interfaces available on the system. */
bool GetNetworkInterfaces(std::list<NetworkInterface> &interfaces);

} // MobiCast namespace 

#endif // !__MOBICAST_NET_H__

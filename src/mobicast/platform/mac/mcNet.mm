/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include <mobicast/mcNet.h>
#include <mobicast/mcDebug.h>
#include <http/HttpSock.h>
#include <ifaddrs.h>

namespace MobiCast
{
    
bool GetNetworkInterfaces(std::list<NetworkInterface> &interfaces)
{
    struct ifaddrs *pIfAddr = NULL;
    
    if(getifaddrs(&pIfAddr) == 0)
    {
        while(pIfAddr != NULL)
        {
            if(pIfAddr->ifa_addr->sa_family == AF_INET)
            {
                NetworkInterface netInt;
                netInt.adapter = pIfAddr->ifa_name;
                netInt.ipv4 = inet_ntoa(((sockaddr_in *)pIfAddr->ifa_addr)->sin_addr);
                interfaces.push_back(netInt);
            }
            pIfAddr = pIfAddr->ifa_next;
        }
        
        freeifaddrs(pIfAddr);
        return true;
    }
    
    return false;
}
    
} // MobiCast namespace

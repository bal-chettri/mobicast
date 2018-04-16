/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/mcNet.h>
#include <mobicast/mcDebug.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "IPHLPAPI.lib")

#define MALLOC(x)       HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x)         HeapFree(GetProcessHeap(), 0, (x))

namespace MobiCast
{

bool GetNetworkInterfaces(std::list<NetworkInterface> &interfaces)
{
    IP_ADAPTER_INFO adapterInfo;
    PIP_ADAPTER_INFO pAdapterInfo = &adapterInfo;

    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
    
    // Allocate memory for adapterInfo if required.
    if(GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        pAdapterInfo = (IP_ADAPTER_INFO *)MALLOC(ulOutBufLen);
        if (pAdapterInfo == NULL) {
            MC_LOGE("mcNet: Error allocating memory for GetAdaptersInfo.");
            return false;
        }
    }

    // Get adapters info and iterate through the list to build
    // the network interface list.
    DWORD dwRetVal = 0;
    if((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
    {
        PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
        while (pAdapter) {
            PIP_ADDR_STRING pIpAddr = &pAdapter->IpAddressList;
            while(pIpAddr)
            {
                NetworkInterface netInt;
                netInt.adapter = pAdapter->Description;
                netInt.ipv4 = pIpAddr->IpAddress.String;
                interfaces.push_back(netInt);
                pIpAddr = pIpAddr->Next;
            }
            pAdapter = pAdapter->Next;
        }
    } else {
        MC_LOGE("mcNet: GetAdaptersInfo failed with error: %d\n", dwRetVal);
    }

    // Free memory for adapters info if allocated.
    if(pAdapterInfo != &adapterInfo) {
        FREE(pAdapterInfo);
    }

    return dwRetVal == 0;
}

} // MobiCast namespace

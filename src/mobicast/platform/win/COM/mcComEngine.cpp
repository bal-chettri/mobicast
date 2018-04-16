/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/platform/win/COM/mcComEngine.h>
#include <mobicast/platform/win/COM/mcComPluginMeta.h>
#include <mobicast/platform/win/COM/mcComUtils.h>
#include <mobicast/mcPathUtils.h>
#include <mobicast/mcDebug.h>
#include <string>
#include <list>

namespace MobiCast
{

const IID *CEngine_IIDs[] =
{
    &IID_IUnknown,
    &IID_IDispatch,
    &IID__Engine,
    NULL
};

CEngine::CEngine(PluginManager *pPluginManager) :
    _pPluginManager(pPluginManager)
{
    MC_ASSERT(_pPluginManager != NULL);
}

CEngine::~CEngine()
{ }

STDMETHODIMP CEngine::get_version(BSTR *pValue)
{
    COM_CHECK_PTR(pValue)

    *pValue = MBS2BSTR(MOBICAST_VERSION, false);

    return S_OK;
}

STDMETHODIMP CEngine::get_debug(VARIANT_BOOL *pValue)
{
    COM_CHECK_PTR(pValue)

#ifdef MC_DEBUG
    *pValue = VARIANT_TRUE;
#else
    *pValue = VARIANT_FALSE;
#endif

    return S_OK;
}

STDMETHODIMP CEngine::get_serviceToken(BSTR *pValue)
{
    COM_CHECK_PTR(pValue)

    extern const char *g_serviceAccessToken;
    *pValue = MBS2BSTR(g_serviceAccessToken, false);

    return S_OK;
}

STDMETHODIMP CEngine::get_pluginCount(int *pRetVal)
{
    COM_CHECK_PTR(pRetVal)

    *pRetVal = (int)_pPluginManager->GetPlugins().size();

    return S_OK;
}

STDMETHODIMP CEngine::getPluginMeta(int index, _PluginMeta **ppRetVal)
{
    const PluginManager::PluginMap &plugins = _pPluginManager->GetPlugins();

    COM_CHECK_ARG(index >= 0 && index < (int)plugins.size())
    COM_CHECK_PTR(ppRetVal)

    PluginManager::PluginMap::const_iterator it = plugins.begin();
    for(int i = 0; i < index; i++) {
        ++it;
    }
    
    *ppRetVal = new CPluginMeta(const_cast<PluginMeta *>(&it->second));

    return S_OK;
}

STDMETHODIMP CEngine::getServices(VARIANT *pRetVal)
{
    COM_CHECK_PTR(pRetVal)

    // Build a list of available services.
    std::list<std::wstring> listServices;
    FindInstalledServices(listServices);
    
    // Create a SAFEARRAY and populate with service entry paths.
    CComArray sa(listServices.size(), false);
    COM_CHECK_MEM((SAFEARRAY *)sa);

    int index = 0;
    for(std::list<std::wstring>::const_iterator it = listServices.begin();
        it != listServices.end(); ++it)
    {
        const std::wstring &str = *it;        

        sa[index].bstrVal = SysAllocString(str.c_str());
        sa[index].vt = VT_BSTR;

        ++index;
    }

    // Return SAFEARRAY of service entry paths.
    pRetVal->vt = VT_ARRAY | VT_VARIANT;
    pRetVal->parray = sa;

    return S_OK;
}

STDMETHODIMP CEngine::log(BSTR msg)
{
    COM_CHECK_ARG(msg)    

    BSTR2MBS strMsg(msg);
    MobiCast::Log("JS", "%s", strMsg);

    return S_OK;
}

void CEngine::FindInstalledServices(std::list<std::wstring> &list) const
{
    // Search for services installed on the system. Services are located
    // at path web/services/ with .jss file extension.
    WIN32_FIND_DATAW findData;
    memset(&findData, 0, sizeof(findData));

    HANDLE hFind = FindFirstFileW(L"web\\services\\*.jss", &findData);
    BOOL found = (hFind != INVALID_HANDLE_VALUE);

    while(found)
    {
        // Make service entry path relative to the virtual directory.
        std::wstring relativePath(L"/services/");
        relativePath.append(findData.cFileName);

        list.push_back(relativePath);

        found = FindNextFileW(hFind, &findData);
    }

    if(hFind != INVALID_HANDLE_VALUE) {
        FindClose(hFind);
    }
}

} // MobiCast namespace

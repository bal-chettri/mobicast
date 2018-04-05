/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_WIN_PLUGIN_META_H__
#define __MOBICAST_PLATFORM_WIN_PLUGIN_META_H__

#include <mobicast/platform/win/COM/mcComBase.h>
#include <mobicast/mcPlugin.h>
#include <mcComLib_i.h>

namespace MobiCast
{

extern const IID *CPluginMeta_IIDs[];

/** CPluginMeta class. Implements scripting PluginMeta object. */
class CPluginMeta : public AutomationComObject<_PluginMeta, CPluginMeta_IIDs, &IID__PluginMeta>
{
public:
    CPluginMeta(PluginMeta *meta);

    STDMETHODIMP get_id(BSTR *pRetVal);
    STDMETHODIMP get_title(BSTR *pRetVal);
    STDMETHODIMP get_description(BSTR *pRetVal);
    STDMETHODIMP get_author(BSTR *pRetVal);
    STDMETHODIMP get_version(BSTR *pRetVal);
    STDMETHODIMP get_player(BSTR *pRetVal);
    STDMETHODIMP get_main(BSTR *pRetVal);
    STDMETHODIMP get_capabilities(int *pRetVal);    
    STDMETHODIMP get_enabled(VARIANT_BOOL *pRetVal);

private:
    PluginMeta *_pPluginMeta;
};

} // MobiCast namespace

#endif // !__MOBICAST_PLATFORM_WIN_PLUGIN_META_H__

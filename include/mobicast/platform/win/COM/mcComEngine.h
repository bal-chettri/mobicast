/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_WIN_ENGINE_H__
#define __MOBICAST_PLATFORM_WIN_ENGINE_H__

#include <mobicast/platform/win/COM/mcComBase.h>
#include <mobicast/mcPlugin.h>
#include <mcComLib_i.h>
#include <list>

namespace MobiCast
{

extern const IID *CEngine_IIDs[];

/** CEngine class. Implements scripting engine object. */
class CEngine : public AutomationComObject<_Engine, CEngine_IIDs, &IID__Engine>
{
public:
    CEngine(PluginManager *pPluginManager);
    virtual ~CEngine();

    // _Engine methods
    STDMETHODIMP get_version(BSTR *pRetVal);
    STDMETHODIMP get_debug(VARIANT_BOOL *pRetVal);
    STDMETHODIMP get_serviceToken(BSTR *pRetVal);
    STDMETHODIMP get_pluginCount(int *pRetVal);

    STDMETHODIMP getPluginMeta(int index, _PluginMeta **ppRetVal);
    STDMETHODIMP getServices(VARIANT *pRetVal);
    STDMETHODIMP log(BSTR msg);

private:
    void FindInstalledServices(std::list<std::wstring> &list) const;

    PluginManager *_pPluginManager;
};

} // MobiCast namespace

#endif // !__MOBICAST_PLATFORM_WIN_ENGINE_H__

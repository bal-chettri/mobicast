/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/platform/win/COM/mcComPluginMeta.h>
#include <mobicast/platform/win/COM/mcComUtils.h>
#include <mobicast/mcDebug.h>

namespace MobiCast
{

const IID *CPluginMeta_IIDs[] =
{
    &IID_IUnknown,
    &IID_IDispatch,
    &IID__PluginMeta,
    NULL
};

CPluginMeta::CPluginMeta(PluginMeta *meta) :
    _pPluginMeta(meta)
{ 
    MC_ASSERT(_pPluginMeta != NULL);
}

STDMETHODIMP CPluginMeta::get_id(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = MBS2BSTR(_pPluginMeta->id.c_str(), false);
    return S_OK;
}

STDMETHODIMP CPluginMeta::get_title(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = MBS2BSTR(_pPluginMeta->title.c_str(), false);
    return S_OK;
}

STDMETHODIMP CPluginMeta::get_description(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = MBS2BSTR(_pPluginMeta->description.c_str(), false);
    return S_OK;
}

STDMETHODIMP CPluginMeta::get_author(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = MBS2BSTR(_pPluginMeta->author.c_str(), false);
    return S_OK;
}

STDMETHODIMP CPluginMeta::get_version(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = MBS2BSTR(_pPluginMeta->version.c_str(), false);
    return S_OK;
}

STDMETHODIMP CPluginMeta::get_player(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = MBS2BSTR(_pPluginMeta->player.c_str(), false);
    return S_OK;
}

STDMETHODIMP CPluginMeta::get_main(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = MBS2BSTR(_pPluginMeta->main.c_str(), false);
    return S_OK;
}

STDMETHODIMP CPluginMeta::get_capabilities(int *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = _pPluginMeta->capabilities;
    return S_OK;
}

STDMETHODIMP CPluginMeta::get_enabled(VARIANT_BOOL *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = _pPluginMeta->enabled ? VARIANT_TRUE : VARIANT_FALSE;
    return S_OK;
}

} // MobiCast namespace

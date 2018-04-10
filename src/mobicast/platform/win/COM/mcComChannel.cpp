/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/platform/win/COM/mcComChannel.h>
#include <mobicast/platform/win/COM/mcComUtils.h>
#include <mobicast/mcDebug.h>

namespace MobiCast
{

const IID *CChannel_IIDs[] =
{
    &IID_IUnknown,
    &IID_IDispatch,
    &IID__Channel,
    NULL
};

CChannel::CChannel(BSTR id, BSTR title) :
    _id(id),
    _title(title)
{ }

CChannel::CChannel(BSTR id, BSTR title, const std::list<_MediaSearch *> &searches) :
    _id(id),
    _title(title),
    _searches(searches)
{ }

CChannel::~CChannel()
{
    SysFreeString(_id);
    SysFreeString(_title);

    // Release references to _MediaSearch objects.
    for(std::list<_MediaSearch *>::iterator it = _searches.begin();
        it != _searches.end();
        ++it)
    {
        (*it)->Release();
    }
}

STDMETHODIMP CChannel::get_id(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = BSTR_COPY(_id);
    return S_OK;
}

STDMETHODIMP CChannel::get_title(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = BSTR_COPY(_title);
    return S_OK;
}

STDMETHODIMP CChannel::get_searches(VARIANT *pRetVal)
{
    COM_CHECK_PTR(pRetVal)

    // Return array of media search objects as SAFEARRAY.
    CComArray sa(_searches.size(), false);
    if((SAFEARRAY *)sa == NULL) {
        return E_OUTOFMEMORY;
    }

    int index = 0;
    for(std::list<_MediaSearch *>::const_iterator it = _searches.begin();
        it != _searches.end(); ++it)
    {
        _MediaSearch *pSearch = *it;

        sa[index].pdispVal = pSearch;
        sa[index].vt = VT_DISPATCH;

        pSearch->AddRef();
        ++index;
    }

    pRetVal->vt = VT_ARRAY | VT_VARIANT;
    pRetVal->parray = sa;

    return S_OK;
}

} // MobiCast namespace

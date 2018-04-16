/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/platform/win/COM/mcComMedia.h>
#include <mobicast/platform/win/COM/mcComUtils.h>
#include <mobicast/mcDebug.h>

namespace MobiCast
{

const IID *CMedia_IIDs[] =
{
    &IID_IUnknown,
    &IID_IDispatch,
    &IID__Media,
    NULL
};

CMedia::CMedia(BSTR type, BSTR title, BSTR mediaUrl, BSTR thumbnailUrl, BSTR duration, BSTR format) :
    _type(type),
    _title(title),
    _mediaUrl(mediaUrl),
    _thumbnailUrl(thumbnailUrl),
    _duration(duration),
    _format(format)
{ }

CMedia::~CMedia()
{
    if(_type != NULL) {
        SysFreeString(_type);
    }
    if(_title != NULL) {
        SysFreeString(_title);
    }
    if(_mediaUrl != NULL) {
        SysFreeString(_mediaUrl);
    }
    if(_thumbnailUrl != NULL) {
        SysFreeString(_thumbnailUrl);
    }
    if(_duration != NULL) {
        SysFreeString(_duration);
    }
    if(_format != NULL) {
        SysFreeString(_format);
    }
}

STDMETHODIMP CMedia::get_type(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = BSTR_COPY(_type);
    return S_OK;
}

STDMETHODIMP CMedia::put_type(BSTR type)
{
    COM_CHECK_ARG(type)
    if(_type != NULL) {
        SysFreeString(_type);
    }
    _type = BSTR_COPY(type);
    return S_OK;
}

STDMETHODIMP CMedia::get_title(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = BSTR_COPY(_title);
    return S_OK;
}

STDMETHODIMP CMedia::get_mediaUrl(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = BSTR_COPY(_mediaUrl);
    return S_OK;
}

STDMETHODIMP CMedia::put_mediaUrl(BSTR url)
{
    COM_CHECK_ARG(url)
    if(_mediaUrl != NULL) {
        SysFreeString(_mediaUrl);
    }
    _mediaUrl = BSTR_COPY(url);
    return S_OK;
}

STDMETHODIMP CMedia::get_thumbnailUrl(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = BSTR_COPY(_thumbnailUrl);
    return S_OK;
}

STDMETHODIMP CMedia::put_thumbnailUrl(BSTR url)
{
    COM_CHECK_ARG(url)
    if(_thumbnailUrl != NULL) {
        SysFreeString(_thumbnailUrl);
    }
    _thumbnailUrl = BSTR_COPY(url);
    return S_OK;
}

STDMETHODIMP CMedia::get_duration(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = BSTR_COPY(_duration);
    return S_OK;
}

STDMETHODIMP CMedia::get_format(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = BSTR_COPY(_format);
    return S_OK;
}

STDMETHODIMP CMedia::put_format(BSTR format)
{
    COM_CHECK_ARG(format)
    if(_format != NULL) {
        SysFreeString(_format);
    }
    _format = BSTR_COPY(format);
    return S_OK;
}

} // MobiCast namespace

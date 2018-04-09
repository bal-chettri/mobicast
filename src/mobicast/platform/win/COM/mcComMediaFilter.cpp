/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/platform/win/COM/mcComMediaFilter.h>
#include <mobicast/platform/win/COM/mcComUtils.h>
#include <mobicast/mcDebug.h>

namespace MobiCast
{

const IID *CMediaFilter_IIDs[] =
{
    &IID_IUnknown,
    &IID_IDispatch,
    &IID__MediaFilter,
    NULL
};

CMediaFilter::CMediaFilter(int size, BSTR dateRange, VARIANT_BOOL subtitles, BSTR type, BSTR quality, int max) :
    _size(size),
    _dateRange(dateRange),
    _subtitles(subtitles),
    _type(type),
    _quality(quality),
    _max(max)
{ }

CMediaFilter::~CMediaFilter()
{
    if(_dateRange != NULL) {
        SysFreeString(_dateRange);
    }
    if(_type != NULL) {
        SysFreeString(_type);
    }
    if(_quality != NULL) {
        SysFreeString(_quality);
    }
}

STDMETHODIMP CMediaFilter::get_size(int *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = _size;
    return S_OK;
}

STDMETHODIMP CMediaFilter::get_dateRange(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = BSTR_COPY(_dateRange);
    return S_OK;
}

STDMETHODIMP CMediaFilter::get_subtitles(VARIANT_BOOL *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = _subtitles;
    return S_OK;
}

STDMETHODIMP CMediaFilter::get_type(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = BSTR_COPY(_type);
    return S_OK;
}

STDMETHODIMP CMediaFilter::get_quality(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = BSTR_COPY(_quality);
    return S_OK;
}

STDMETHODIMP CMediaFilter::get_max(int *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = _max;
    return S_OK;
}

static void AppendFilter(std::string &str_filter, const char *format, const char *value)
{
    if(value != NULL) {
        char szFilter[100];
        if(!str_filter.empty()) {
            str_filter.append(",");
        }
        sprintf(szFilter, format, value);
        str_filter.append(szFilter);
    }
}

static void AppendFilter(std::string &str_filter, const char *format, int value)
{
    if(value != 0) {
        char szFilter[100];
        if(!str_filter.empty()) {
            str_filter.append(",");
        }
        sprintf(szFilter, format, value);
        str_filter.append(szFilter);
    }
}

std::string &CMediaFilter::ToString(std::string &strFilters) const
{
    strFilters.clear();

    const char *szDateRange = _dateRange == NULL ? NULL : (const char *)BSTR2MBS(_dateRange, false);
    const char *szType = _type == NULL ? NULL : (const char *)BSTR2MBS(_type, false);
    const char *szQuality = _quality == NULL ? NULL : (const char *)BSTR2MBS(_quality, false);

    AppendFilter(strFilters, "size=%d", _size);
    AppendFilter(strFilters, "max=%d", _max);
    AppendFilter(strFilters, "subtitles=%s", _subtitles == VARIANT_TRUE ? "true" : "false");
    AppendFilter(strFilters, "date-range=%s", szDateRange);
    AppendFilter(strFilters, "type=%s", szType);
    AppendFilter(strFilters, "quality=%s", szQuality);

    if(szDateRange != NULL) {
        delete [] szDateRange;
    }
    if(szType != NULL) {
        delete [] szType;
    }
    if(szQuality != NULL) {
        delete [] szQuality;
    }

    return strFilters;
}

} // MobiCast namespace

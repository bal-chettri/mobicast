/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_WIN_MEDIA_FILTER_H__
#define __MOBICAST_PLATFORM_WIN_MEDIA_FILTER_H__

#include <mobicast/platform/win/COM/mcComBase.h>
#include <mcComLib_i.h>
#include <string>

namespace MobiCast
{

extern const IID *CMediaFilter_IIDs[];

/** CMediaFilter class. Implements scripting _MediaFilter interface. */
class CMediaFilter : public AutomationComObject<_MediaFilter, CMediaFilter_IIDs, &IID__MediaFilter>
{
public:
    CMediaFilter(int size, BSTR dateRange, VARIANT_BOOL subtitles, BSTR type, BSTR quality, int max);
    virtual ~CMediaFilter();

    STDMETHODIMP get_size(int *pRetVal);
    STDMETHODIMP get_dateRange(BSTR *pRetVal);
    STDMETHODIMP get_subtitles(VARIANT_BOOL *pRetVal);
    STDMETHODIMP get_type(BSTR *pRetVal);
    STDMETHODIMP get_quality(BSTR *pRetVal);
    STDMETHODIMP get_max(int *pRetVal);

    // Public internal methods not exposed to COM.
    std::string &ToString(std::string &str) const;

private:
    int             _size;
    BSTR            _dateRange;
    VARIANT_BOOL    _subtitles;
    BSTR            _type;
    BSTR            _quality;
    int             _max;
};

} // MobiCast namespace

#endif // !__MOBICAST_PLATFORM_WIN_MEDIA_FILTER_H__

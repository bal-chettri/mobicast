/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_WIN_MEDIA_H__
#define __MOBICAST_PLATFORM_WIN_MEDIA_H__

#include <mobicast/platform/win/COM/mcComBase.h>
#include <mcComLib_i.h>

namespace MobiCast
{

extern const IID *CMedia_IIDs[];

/** _Media class. Implements scripting Media object. */
class CMedia : public AutomationComObject<_Media, CMedia_IIDs, &IID__Media>
{
public:
    CMedia(BSTR type, BSTR title, BSTR mediaUrl, BSTR thumbnailUrl, BSTR duration, BSTR format);
    virtual ~CMedia();

    STDMETHODIMP get_type(BSTR *pRetVal);
    STDMETHODIMP put_type(BSTR type);
    STDMETHODIMP get_title(BSTR *pRetVal);
    STDMETHODIMP get_mediaUrl(BSTR *pRetVal);
    STDMETHODIMP put_mediaUrl(BSTR url);
    STDMETHODIMP get_thumbnailUrl(BSTR *pRetVal);
    STDMETHODIMP put_thumbnailUrl(BSTR url);
    STDMETHODIMP get_duration(BSTR *pRetVal);
    STDMETHODIMP get_format(BSTR *pRetVal);
    STDMETHODIMP put_format(BSTR format);

    // Public internal methods not exposed to COM.
    inline BSTR GetType() const { return _type; }
    inline BSTR GetTitle() const { return _title; }
    inline BSTR GetMediaUrl() const { return _mediaUrl; }
    inline BSTR GetThumbnailUrl() const { return _thumbnailUrl; }
    inline BSTR GetDuration() const { return _duration; }
    inline BSTR GetFormat() const { return _format; }

private:
    BSTR            _type;
    BSTR            _title;
    BSTR            _mediaUrl;
    BSTR            _thumbnailUrl;
    BSTR            _duration;
    BSTR            _format;
};

} // MobiCast namespace

#endif // !__MOBICAST_PLATFORM_WIN_MEDIA_H__

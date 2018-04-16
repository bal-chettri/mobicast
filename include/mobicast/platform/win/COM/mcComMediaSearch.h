/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_WIN_MEDIA_SEARCH_H__
#define __MOBICAST_PLATFORM_WIN_MEDIA_SEARCH_H__

#include <mobicast/platform/win/COM/mcComBase.h>
#include <mobicast/platform/win/COM/mcComMediaFilter.h>
#include <mobicast/mcHtmlStrainer.h>
#include <mcComLib_i.h>
#include <list>

namespace MobiCast
{

extern const IID *CMediaSearch_IIDs[];

// Forward declarations.
class CMedia;

struct MediaSearchContext;

/** CMediaSearch class. Implements scripting MediaSearch object. */
class CMediaSearch : public AutomationComObject<_MediaSearch, CMediaSearch_IIDs, &IID__MediaSearch>
{
public:
    CMediaSearch(BSTR id, BSTR source, BSTR keywords, _MediaFilter *filter);
    virtual ~CMediaSearch();

    // _MediaSearch methods
    STDMETHODIMP get_id(BSTR *pRetVal);
    STDMETHODIMP get_mediaSource(BSTR *pRetVal);
    STDMETHODIMP get_keywords(BSTR *pRetVal);
    STDMETHODIMP get_filter(_MediaFilter **pRetVal);

    // Public internal methods not exposed to COM.

    inline BSTR GetId() const { return _id; }
    inline BSTR GetSource() const { return _source; }
    inline BSTR GetKeywords() const { return _keywords; }
    inline _MediaFilter *GetFilter() const { return _filter; }
    inline std::string GetFilterAsString() const
    {
        std::string strFilters;
        if(_filter != NULL) {
            static_cast<CMediaFilter *>(_filter)->ToString(strFilters);
        }
        return strFilters;
    }

    /** Performs the search and appends the result to `mediaList` object. */
    bool Perform(std::list<CMedia *> &mediaList);

private:
    /** Gets the search URL from the plugin's media source. */
    bool GetSearchUrl(VARIANT &bstrUrl);

    /** Gets the media extraction tags array from the plugin's media source. */
    bool GetMediaExtractionTags(VARIANT &varMediaTags);

    /** Notifies the plugin's media source about the media item found as a result of search. */
    void NotifyMediaItem(_Media *pMedia);

    /** Initializes HTML strainer for a media field before a search. */
    bool InitStrainer(VARIANT &varFieldTags, HtmlStrainer &expr);

    /** Performs a web search for media items. */
    bool PerformSearch(BSTR bstrSearchUrl, MediaSearchContext *pContext);

    static size_t SearchWriteCallback(char* buf, size_t size, size_t nmemb, void* up);

    BSTR            _id;
    BSTR            _source;
    BSTR            _keywords;
    _MediaFilter    *_filter;
};

} // MobiCast namespace

#endif // !__MOBICAST_PLATFORM_WIN_MEDIA_SEARCH_H__

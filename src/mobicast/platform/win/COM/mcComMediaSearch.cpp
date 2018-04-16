/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/platform/win/COM/mcComMediaSearch.h>
#include <mobicast/platform/win/COM/mcComMedia.h>
#include <mobicast/platform/win/COM/mcComUtils.h>
#include <mobicast/platform/win/mcWindow.h>
#include <mobicast/mcDebug.h>
#include <curl/curl.h>

extern MobiCast::Window *g_window;

namespace MobiCast
{

const IID *CMediaSearch_IIDs[] =
{
    &IID_IUnknown,
    &IID_IDispatch,
    &IID__MediaSearch,
    NULL
};

//
// MediaSearchContext struct.
// MediaSearchContext struct contains contextual data for a single search operation.
//
struct MediaSearchContext
{
    // Media list where to append the search results.
    std::list<CMedia *> *pMediaList;

    // Filter for media content url.
    HtmlStrainer *pExprMediaUrl;

    // Filter for media thumbnail url.
    HtmlStrainer *pExprThumbnail;

    // Filter for media duration.
    HtmlStrainer *pExprDuration;

    // Filter for media title.
    HtmlStrainer *pExprTitle;
};

//
// CMediaSearch class definition.
//
CMediaSearch::CMediaSearch(BSTR id, BSTR source, BSTR keywords, _MediaFilter *filter) :
    _id(id),
    _source(source),
    _keywords(keywords),
    _filter(filter)
{
    if(_filter != NULL) {
        _filter->AddRef();
    }
}

CMediaSearch::~CMediaSearch()
{
    if(_id != NULL) {
        SysFreeString(_id);
    }
    SysFreeString(_source);
    SysFreeString(_keywords);
    if(_filter != NULL) {
        _filter->Release();
    }
}

STDMETHODIMP CMediaSearch::get_id(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = BSTR_COPY(_id);
    return S_OK;
}

STDMETHODIMP CMediaSearch::get_mediaSource(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = BSTR_COPY(_source);
    return S_OK;
}

STDMETHODIMP CMediaSearch::get_keywords(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = BSTR_COPY(_keywords);
    return S_OK;
}

STDMETHODIMP CMediaSearch::get_filter(_MediaFilter **ppRetVal)
{
    COM_CHECK_PTR(ppRetVal)
    *ppRetVal = _filter;
    if(*ppRetVal != NULL) {
        (*ppRetVal)->AddRef();
    }
    return S_OK;
}

bool CMediaSearch::Perform(std::list<CMedia *> &mediaList)
{
    // Get search URL from the plugin.
    VARIANT bstrSearchUrl;
    if(!GetSearchUrl(bstrSearchUrl)) {
        return false;
    }

    // Get media extraction tags from the plugin.
    VARIANT varMediaTags;
    if(!GetMediaExtractionTags(varMediaTags)) {
        VariantClear(&bstrSearchUrl);
        return false;
    }

    // Get media tags array for each field to extract. Media tags must have this format:
    // [ videoUrlTags, thumbnailTags, durationTags, titleTags ]
    VARIANT *pMediaTags = NULL;
    SafeArrayAccessData(varMediaTags.parray, reinterpret_cast<void **>(&pMediaTags));

    // Initialize strainers to extract media fields from search response.
    HtmlStrainer matchExprs[4];
    for(int i = 0; i < sizeof(matchExprs) / sizeof(matchExprs[0]); i++)
    {
        if(!InitStrainer(pMediaTags[i], matchExprs[i]))
        {
            VariantClear(&varMediaTags);
            VariantClear(&bstrSearchUrl);
            return false;
        }
    }

    VariantClear(&varMediaTags);

    // Build context for performing the search.
    MediaSearchContext context;
    context.pMediaList = &mediaList;
    context.pExprMediaUrl = matchExprs + 0;
    context.pExprThumbnail = matchExprs + 1;
    context.pExprDuration = matchExprs + 2;
    context.pExprTitle = matchExprs + 3;

    // Finally, perform the search using search url and context.
    bool ret = PerformSearch(bstrSearchUrl.bstrVal, &context);

    VariantClear(&bstrSearchUrl);

    return ret;
}

bool CMediaSearch::GetSearchUrl(VARIANT &bstrUrl)
{
    VariantInit(&bstrUrl);

    VARIANT args[3];
    args[2].vt = VT_BSTR;
    args[2].bstrVal = _source;
    args[1].vt = VT_BSTR;
    args[1].bstrVal = _keywords;
    args[0].vt = VT_DISPATCH;
    args[0].pdispVal = _filter;

    bool invokeStatus = g_window->GetBrowser()->InvokeMethod(
        L"_mc_js_plugin_get_search_url", args, 3, bstrUrl);

    MC_ASSERT(invokeStatus);

    if(bstrUrl.vt != VT_BSTR) {
        VariantClear(&bstrUrl);
        return false;
    }

    return true;
}

bool CMediaSearch::GetMediaExtractionTags(VARIANT &varMediaTags)
{
    VariantInit(&varMediaTags);

    VARIANT args[1];
    args[0].vt = VT_BSTR;
    args[0].bstrVal = _source;

    bool invokeStatus = g_window->GetBrowser()->InvokeMethod(
        L"_mc_js_plugin_get_media_extraction_tags", args, 1, varMediaTags);

    MC_ASSERT(invokeStatus);

    if(varMediaTags.vt != (VT_ARRAY | VT_VARIANT) || SafeArrayGetDim(varMediaTags.parray) != 1) {
        VariantClear(&varMediaTags);
        return false;
    }

    // Validate the dimensions of media tags array.
    //
    // A valid media tags array must have following structure starting at index 0:
    // [ videoUrlTags:[], thumbnailTags:[], durationTags:[], titleTags:[] ]

    LONG ldimMediaTags, udimMediaTags;
    SafeArrayGetLBound(varMediaTags.parray, 1, &ldimMediaTags);
    SafeArrayGetUBound(varMediaTags.parray, 1, &udimMediaTags);

    if(ldimMediaTags != 0 || udimMediaTags != 3) {
        VariantClear(&varMediaTags);
        return false;
    }

    return true;
}

void CMediaSearch::NotifyMediaItem(_Media *pMedia)
{
    VARIANT ret, args[2];

    VariantInit(&ret);

    args[1].vt = VT_BSTR;
    args[1].bstrVal = _source;
    args[0].vt = VT_DISPATCH;
    args[0].pdispVal = pMedia;

    bool invokeStatus = g_window->GetBrowser()->InvokeMethod(
        L"_mc_js_plugin_on_media_item_found", args, 2, ret);

    MC_ASSERT(invokeStatus);

    VariantClear(&ret);
}

bool CMediaSearch::InitStrainer(VARIANT &varFieldTags, HtmlStrainer &expr)
{
    if(varFieldTags.vt != (VT_VARIANT | VT_ARRAY)) {
        MC_LOG_DEBUG("Invalid media FieldTags array.");
        return false;
    }

    std::list<std::string> listTags;

    CComArray arrTags(varFieldTags.parray);
    for(int i = 0; i < (int)arrTags.GetCount(); i++)
    {
        if(arrTags[i].vt != VT_BSTR) {
            return false;
        }

        BSTR2MBS tag(arrTags[i].bstrVal);
        listTags.push_back((const char *)tag);
    }

    expr.Init(listTags);
    expr.SetRepeat(true);

    return true;
}

bool CMediaSearch::PerformSearch(BSTR bstrSearchURL, MediaSearchContext *pContext)
{
    // Create a CURL HTTP request to search media from specified URL.
    CURL *curl = curl_easy_init();
    if(!curl) {
        return false;
    }

    BSTR2MBS searchURL(bstrSearchURL);
    curl_easy_setopt(curl, CURLOPT_URL, (const char *)searchURL);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &CMediaSearch::SearchWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)pContext);

    CURLcode curlRet = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if(curlRet != CURLE_OK) {
        MC_LOG_ERR("curl_easy_perform() failed: %s", curl_easy_strerror(curlRet));
        return false;
    }

    // Get the filtered texts to populate the media list.
    const std::list<std::string> &urlTexts = pContext->pExprMediaUrl->GetTexts();
    const std::list<std::string> &thumbnailTexts = pContext->pExprThumbnail->GetTexts();
    const std::list<std::string> &durationTexts = pContext->pExprDuration->GetTexts();
    const std::list<std::string> &titleTexts = pContext->pExprTitle->GetTexts();

    // Length of texts for each field must match with others.
    size_t length = urlTexts.size();
    if(length != thumbnailTexts.size() || length != durationTexts.size() || length != titleTexts.size()) {
        MC_LOG_ERR("Invalid length of texts in one or more match expressions.");
        return false;
    }

    // Iterate through texts to build media object.
    std::list<std::string>::const_iterator itURL = urlTexts.begin();
    std::list<std::string>::const_iterator itThumbnail = thumbnailTexts.begin();
    std::list<std::string>::const_iterator itDuration = durationTexts.begin();
    std::list<std::string>::const_iterator itTitle = titleTexts.begin();

    while(itURL != urlTexts.end())
    {
        // Create a Media object. References are transfered to the object so autoFree is false.
        MBS2BSTR bstrTitle(itTitle->c_str(), false);
        MBS2BSTR bstrMediaUrl(itURL->c_str(), false);
        MBS2BSTR bstrThumbUrl(itThumbnail->c_str(), false);
        MBS2BSTR bstrDuration(itDuration->c_str(), false);

        CMedia *pMedia = new CMedia(NULL, bstrTitle, bstrMediaUrl, bstrThumbUrl, bstrDuration, NULL);

        // Tell the plugin's media source a media item was found. The source
        // might want to update some properties of the media, like video url.
        NotifyMediaItem(pMedia);

        // Append the media item to the media list.
        pContext->pMediaList->push_back(pMedia);

        ++itURL;
        ++itThumbnail;
        ++itDuration;
        ++itTitle;
    }

    return true;
}

size_t CMediaSearch::SearchWriteCallback(char* buf, size_t size, size_t nmemb, void* up)
{
    MediaSearchContext *pContext = reinterpret_cast<MediaSearchContext *>(up);

    // Pour the buffer to media filters extracting the required texts.
    pContext->pExprMediaUrl->Pour(buf, size * nmemb);
    pContext->pExprThumbnail->Pour(buf, size * nmemb);
    pContext->pExprDuration->Pour(buf, size * nmemb);
    pContext->pExprTitle->Pour(buf, size * nmemb);

    return size * nmemb;
}

} // MobiCast namespace

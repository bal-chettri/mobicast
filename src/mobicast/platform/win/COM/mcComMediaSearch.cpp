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
#include <map>

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
    BSTR jsCallback;
    VARIANT jsContext;
    std::map<HtmlStrainer *, std::string> htmlFilters;
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

STDMETHODIMP CMediaSearch::execute(BSTR url, VARIANT contentTags, BSTR jsCallback, VARIANT jsContext)
{
    COM_CHECK_ARG(url);
    COM_CHECK_ARG(jsCallback);

    MC_LOGD("CMediaSearch::execute");

    COM_CHECK_ARG(contentTags.vt == VT_DISPATCH);
    CJsValue jsContentTags(contentTags);

    MC_ASSERT(jsContentTags.IsObject());
    jsContentTags.PrefetchKeyValues();

    MediaSearchContext searchContext;
    searchContext.jsCallback = jsCallback;

    VariantInit(&searchContext.jsContext);
    VariantCopy(&searchContext.jsContext, &jsContext);

    const CJsValue::KeyValueMap &keyValues = jsContentTags.GetKeyValueMap();
    for(CJsValue::KeyValueMap::const_iterator it = keyValues.begin();
        it != keyValues.end();
        ++it)
    {
        const std::string &strFieldName = it->first;
        CJsValue arrTags(it->second);

        HtmlStrainer *pHtmlStrainer = new HtmlStrainer();
        if(!InitStrainer(arrTags, *pHtmlStrainer)) {
            delete pHtmlStrainer;
            return false;
        }
        searchContext.htmlFilters[pHtmlStrainer] = strFieldName;
    }

    // Finally, perform the search using search url and context.
    bool searchStatus = PerformSearch(url, &searchContext);

    for(std::map<HtmlStrainer *, std::string>::iterator it = searchContext.htmlFilters.begin(); 
        it != searchContext.htmlFilters.end();
        ++it)
    {
        delete it->first;
    }

    VariantClear(&searchContext.jsContext);

    if(searchStatus)
    {
        return S_OK;
    }

    return S_FALSE;
}

bool CMediaSearch::InitStrainer(CJsValue &varFieldTags, HtmlStrainer &expr)
{
    if(!varFieldTags.IsArray()) {
        MC_LOG_DEBUG("Invalid media FieldTags array.");
        return false;
    }

    std::list<std::string> listTags;

    int length = varFieldTags.GetArrayLength();
    varFieldTags.PrefetchKeyValues();

    for(int i = 0; i < length; i++)
    {
        char szIndex[10];
        itoa(i, szIndex, 10);
        CJsValue value = varFieldTags.GetValueForKey(szIndex);

        if(!value.IsString()) {
            return false;
        }

        std::string strTag = value.StringValue();
        listTags.push_back(strTag);
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

#if 0
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    // headers = curl_slist_append(headers, "Accept-Encoding: gzip, deflate");
    headers = curl_slist_append(headers, "Accept-Language: en-US,en;q=0.5");
    headers = curl_slist_append(headers, "Upgrade-Insecure-Requests: 1");
    headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 6.1; rv:61.0) Gecko/20100101 Firefox/61.0");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
#endif

    CURLcode curlRet = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if(curlRet != CURLE_OK) {
        MC_LOG_ERR("curl_easy_perform() failed: %s", curl_easy_strerror(curlRet));
        return false;
    }

    int length;
    std::map<HtmlStrainer *, std::string>::iterator it = pContext->htmlFilters.begin();
    if(it == pContext->htmlFilters.end()) {
        length = 0;
    } else {
        length = (int)it->first->GetTexts().size();
    }

    for(int index = 0; index < length; index++)
    {
        for(std::map<HtmlStrainer *, std::string>::iterator it = pContext->htmlFilters.begin();
            it != pContext->htmlFilters.end();
            ++it)
        {
            HtmlStrainer *htmlFilter = it->first;
            const std::string &tagName = it->second;

            const std::vector<std::string> &texts = htmlFilter->GetTexts();
            MC_ASSERT(index < (int)texts.size());

            const std::string &text = texts.at(index);

            MBS2BSTR bstrTagName(tagName.c_str());
            MBS2BSTR bstrText(text.c_str());

            VARIANT args[3];
            args[2].vt = VT_BSTR;
            args[2].bstrVal = bstrTagName;
            args[1].vt = VT_BSTR;
            args[1].bstrVal = bstrText;
            VariantInit(&args[0]);
            VariantCopy(&args[0], &pContext->jsContext);
    
            VARIANT result;
            VariantInit(&result);

            bool invokeStatus = g_window->GetBrowser()->InvokeMethod(
                pContext->jsCallback, args, 3, result);

            VariantClear(&result);
            VariantClear(&args[0]);

            MC_ASSERT(invokeStatus);
        }
    }

    return true;
}

size_t CMediaSearch::SearchWriteCallback(char* buf, size_t size, size_t nmemb, void* up)
{
    MediaSearchContext *pContext = reinterpret_cast<MediaSearchContext *>(up);

    // Pour the buffer to media filters extracting the required texts.
    for(std::map<HtmlStrainer *, std::string>::iterator it = pContext->htmlFilters.begin(); 
        it != pContext->htmlFilters.end();
        ++it)
    {
        HtmlStrainer *htmlFilter = it->first;
        htmlFilter->Pour(buf, size * nmemb);
    }

    return size * nmemb;
}

} // MobiCast namespace

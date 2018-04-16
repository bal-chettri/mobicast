/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/platform/win/COM/mcComMediaManager.h>
#include <mobicast/platform/win/COM/mcComMedia.h>
#include <mobicast/platform/win/COM/mcComMediaFilter.h>
#include <mobicast/platform/win/COM/mcComMediaSearch.h>
#include <mobicast/platform/win/COM/mcComChannel.h>
#include <mobicast/platform/win/COM/mcComUtils.h>
#include <mobicast/mcDebug.h>

namespace MobiCast
{

const IID *CMediaManager_IIDs[] =
{
    &IID_IUnknown,
    &IID_IDispatch,
    &IID__MediaManager,
    NULL
};

// Database key prefix for registering media handlers. Handler for .mp4 media
// format, for e.g., is stored under key _MEDIA_HANDLER_.mp4
#define kDbKeyMediaHandler      "_MEDIA_HANDLER_"

// Context for GetChannel.
struct GetChannelContext
{
    BSTR                        title;
    std::list<_MediaSearch *>   searches;
};

// Context for GetChannels.
struct GetChannelsContext
{
    std::list<_Channel *> channels;
};

//
// CMediaManager class definition.
//
CMediaManager::CMediaManager(Database *db) :
    _db(db)
{ }

CMediaManager::~CMediaManager()
{ }

STDMETHODIMP CMediaManager::getChannel(BSTR bstrChannelId, _Channel **ppRetVal)
{
    COM_CHECK_ARG(bstrChannelId)
    COM_CHECK_PTR(ppRetVal)

    rowid_t nChannelId = _wtoi64(bstrChannelId);

    // Get channel info.
    GetChannelContext context;
    context.title = NULL;
    _db->GetChannel(nChannelId, (Database::RowCallback)GetChannelCallback, &context);

    if(context.title == NULL) {
        *ppRetVal = NULL;
        return S_FALSE;
    }

    // Get channel searches.
    _db->GetChannelSearches(nChannelId, (Database::RowCallback)GetChannelSearchesCallback, &context);

    bstrChannelId = BSTR_COPY(bstrChannelId);

    *ppRetVal = new CChannel(bstrChannelId, context.title, context.searches);

    return S_OK;
}

STDMETHODIMP CMediaManager::getChannels(VARIANT *pRetVal)
{
    COM_CHECK_PTR(pRetVal)

    // Get channel list.
    GetChannelsContext context;
    _db->GetChannels((Database::RowCallback)GetChannelsCallback, &context);

    // Create SAFEARRAY of channel objects.
    CComArray arrChannels(context.channels.size(), false);
    MC_ASSERT((SAFEARRAY *)arrChannels != NULL);

    // Build SAFEARRAY of channels from the list.
    int index = 0;
    for(std::list<_Channel *>::const_iterator it = context.channels.begin();
        it != context.channels.end(); ++it)
    {
        _Channel *pChannel = *it;
        arrChannels[index].pdispVal = pChannel;
        arrChannels[index].vt = VT_DISPATCH;
        ++index;
    }
    pRetVal->vt = VT_ARRAY | VT_VARIANT;
    pRetVal->parray = arrChannels;

    return S_OK;
}

STDMETHODIMP CMediaManager::createSearchFilter(int size, BSTR dateRange, VARIANT_BOOL subtitles,
                                                BSTR type, BSTR quality, int max,
                                                _MediaFilter **ppRetVal)
{
    COM_CHECK_PTR(ppRetVal)

    dateRange = BSTR_COPY(dateRange);
    type = BSTR_COPY(type);
    quality = BSTR_COPY(quality);

    *ppRetVal = new CMediaFilter(size, dateRange, subtitles, type, quality, max);
    return S_OK;
}

STDMETHODIMP CMediaManager::createSearch(BSTR source, BSTR keywords, IDispatch *pFilter,
                                        _MediaSearch **ppRetVal)
{
    COM_CHECK_PTR(ppRetVal)

    _MediaFilter *pFilterObj;
    if(FAILED(pFilter->QueryInterface(IID__MediaFilter, (void **)&pFilterObj))) {
        return S_FALSE;
    }

    source = BSTR_COPY(source);
    keywords = BSTR_COPY(keywords);

    *ppRetVal = new CMediaSearch(NULL, source, keywords, pFilterObj);

    pFilterObj->Release();
    return S_OK;
}

STDMETHODIMP CMediaManager::addChannel(BSTR title, VARIANT searches, BSTR *pRetVal)
{
    COM_CHECK_ARG(title)
    COM_CHECK_ARG(searches.vt == (VT_ARRAY | VT_VARIANT))
    COM_CHECK_PTR(pRetVal)

    // Begin a transaction since we need to create multiple records.
    _db->BeginTransaction();

    HRESULT hr = S_OK;
    std::list<rowid_t> listSearchIds;

    // Iterate through the search array and insert search records.
    CComArray arrSearches(searches.parray);
    for(int i = 0; i < (int)arrSearches.GetCount(); i++)
    {
        VARIANT *pItem = &arrSearches[i];

        _MediaSearch *pMediaSearch;

        if(pItem->vt != VT_DISPATCH ||
           FAILED(pItem->pdispVal->QueryInterface(IID__MediaSearch, (void **)&pMediaSearch)))
        {
            hr = S_FALSE;
            break;
        }

        CMediaSearch *pMediaSearchObj = static_cast<CMediaSearch *>(pMediaSearch);

        BSTR2MBS strSource(pMediaSearchObj->GetSource());
        BSTR2MBS strKeywords(pMediaSearchObj->GetKeywords());
        std::string strFilters = pMediaSearchObj->GetFilterAsString();

        pMediaSearch->Release();

        // Insert a Search record to db and append its ID to the list.
        rowid_t searchId = _db->AddSearch(strSource, strKeywords, strFilters.c_str());
        if(searchId == -1) {
            hr = S_FALSE;
            break;
        }

        listSearchIds.push_back(searchId);
    }

    // Rollback the transaction and return if an error occurred.
    if(hr != S_OK)
    {
        _db->RollbackTransaction();
        return hr;
    }

    // Insert a new Channel record to db.
    BSTR2MBS strTitle(title);
    rowid_t channelId = _db->AddChannel(strTitle);
    if(channelId == -1)
    {
        _db->RollbackTransaction();
        return S_FALSE;
    }

    // Now, link the search records to the new channel.
    for(std::list<rowid_t>::const_iterator it = listSearchIds.begin();
        it != listSearchIds.end(); ++it)
    {
        const rowid_t searchId = *it;
        if(_db->AddChannelSearch(channelId, searchId) == -1)
        {
            hr = S_FALSE;
            break;
        }
    }

    // Rollback the transaction and return if an error occurred.
    if(hr != S_OK)
    {
        _db->RollbackTransaction();
        return hr;
    }

    // Commit the transaction and return the new channel id.
    _db->CommitTransaction();

    wchar_t wszChannelId[50];
    _swprintf(wszChannelId, L"%lld", channelId);
    *pRetVal = SysAllocString(wszChannelId);

    return S_OK;
}

STDMETHODIMP CMediaManager::listChannel(BSTR channelId, VARIANT *pRetVal)
{
    COM_CHECK_ARG(channelId)
    COM_CHECK_PTR(pRetVal)

    // Get the channel object.
    _Channel *pChannel;
    HRESULT hr = getChannel(channelId, &pChannel);
    if(FAILED(hr) || pChannel == NULL) {
        return hr;
    }

    // List of media to return as a result of channel search.
    std::list<CMedia *> mediaList;

    // Perform all searches in the channel.
    const std::list<_MediaSearch *> &searches = static_cast<CChannel *>(pChannel)->GetSearches();
    for(std::list<_MediaSearch *>::const_iterator it = searches.begin(); it != searches.end(); ++it)
    {
        CMediaSearch *pSearch = static_cast<CMediaSearch *>(*it);
        pSearch->Perform(mediaList);
    }

    pChannel->Release();

    // Return array of _Media items as SAFEARRAY.
    SAFEARRAY *psaMediaItems = MediaListToSafeArray(mediaList);

    if(psaMediaItems == NULL)
    {
        // Release local references in mediaList.
        for(std::list<CMedia *>::const_iterator it = mediaList.begin(); it != mediaList.end(); ++it) {
            (*it)->Release();
        }
        return E_OUTOFMEMORY;
    }

    pRetVal->vt = VT_ARRAY | VT_VARIANT;
    pRetVal->parray = psaMediaItems;

    return S_OK;
}

STDMETHODIMP CMediaManager::registerMediaHandler(BSTR format, BSTR pluginId)
{
    COM_CHECK_ARG(format && wcslen(format) > 1 && format[0] == L'.');
    COM_CHECK_ARG(pluginId);

    BSTR2MBS strFormat(format);
    BSTR2MBS strPluginId(pluginId);

    // Register the media handler under key _MEDIA_HANDLER_{.format}
    std::string strKey(kDbKeyMediaHandler);
    strKey.append(strFormat);

    if(_db->SetProperty(strKey.c_str(), strPluginId, Database::kValueTypeText))
    {
        return S_OK;
    }

    return S_FALSE;
}

STDMETHODIMP CMediaManager::listMediaHandlers(VARIANT *pRetVal)
{
    COM_CHECK_PTR(pRetVal);

    VariantInit(pRetVal);

    // Get all db properties and check for media handler key prefix.
    std::map<std::string, Database::TypedValue> props;
    _db->GetAllProperties(props);

    size_t count = 0;
    std::map<std::string, Database::TypedValue>::const_iterator it;
    for(it = props.begin(); it != props.end(); ++it)
    {
        std::string key = it->first;
        if(key.find(kDbKeyMediaHandler) == 0)
        {
            ++count;
        }
    }

    // Create and return list of media handlers as a SAFEARRAY.
    CComArray arr(count, false);
    int index = 0;
    for(it = props.begin(); it != props.end(); ++it)
    {
        std::string key = it->first;
        if(key.find(kDbKeyMediaHandler) == 0)
        {
            // Format handler info string as .format/plugin.
            std::string handler = key.substr(strlen(kDbKeyMediaHandler));
            handler.append("/");
            handler.append(it->second.value);

            arr[index].vt = VT_BSTR;
            arr[index].bstrVal = MBS2BSTR(handler.c_str(), false);

            ++index;
        }
    }

    pRetVal->vt = VT_ARRAY | VT_VARIANT;
    pRetVal->parray = arr;

    return S_OK;
}

SAFEARRAY *CMediaManager::MediaListToSafeArray(const std::list<CMedia *> &list)
{
    // Create a SAFEARRAY of _Media items with VT_VARAINT as element type.
    CComArray arrMediaItems(list.size(), false);
    if((SAFEARRAY *)arrMediaItems == NULL) {
        return NULL;
    }

    int index = 0;
    for(std::list<CMedia *>::const_iterator it = list.begin(); it != list.end(); ++it)
    {
        arrMediaItems[index].vt = VT_DISPATCH;
        arrMediaItems[index].pdispVal = *it;
        ++index;
    }

    return arrMediaItems;
}

_MediaFilter *CMediaManager::CreateMediaFilter(const char *szFilter)
{
    int size = 0;
    BSTR dateRange = NULL;
    BSTR type = NULL;
    BSTR quality = NULL;
    VARIANT_BOOL subtitles = VARIANT_FALSE;
    int max = 0;

    char *filters = strdup(szFilter);
    char *filter = strtok(filters, ",");

    while(filter != NULL)
    {
        const char *delim = strchr(filter, '=');
        MC_ASSERT(delim != NULL);

        std::string name;
        const char *value;

        name.assign(filter, delim - filter);
        value = delim + 1;

        if(name == "size") {
            size = atoi(value);
        } else if(name == "date-range") {
            dateRange = MBS2BSTR(value, false);
        } else if(name == "subtitles") {
            subtitles = strcmp(value, "true") ? VARIANT_FALSE : VARIANT_TRUE;
        } else if(name == "type") {
            type = MBS2BSTR(value, false);
        } else if(name == "quality") {
            quality = MBS2BSTR(value, false);
        } else if(name == "max") {
            max = atoi(value);
        }

        filter = strtok(NULL, ",");
    }

    free(filters);

    return new CMediaFilter(size, dateRange, subtitles, type, quality, max);
}

int CMediaManager::GetChannelCallback(const Database::ChannelRow *row, void *context)
{
    GetChannelContext *pContext = reinterpret_cast<GetChannelContext *>(context);
    MC_ASSERT(pContext != NULL);

    pContext->title = MBS2BSTR(row->title, false);

    return 0;
}

int CMediaManager::GetChannelsCallback(const Database::ChannelRow *row, void *context)
{
    GetChannelsContext *pContext = reinterpret_cast<GetChannelsContext *>(context);
    MC_ASSERT(pContext != NULL);

    char szId[50];
    sprintf(szId, "%lld", row->id);
    BSTR id = MBS2BSTR(szId, false);

    BSTR title = MBS2BSTR(row->title, false);

    // Create and append the channel to the channel list.
    CChannel *channel = new CChannel(id, title);
    pContext->channels.push_back(channel);

    return 0;
}

int CMediaManager::GetChannelSearchesCallback(const Database::ChannelSearchRow *row, void *context)
{
    GetChannelContext *pContext = reinterpret_cast<GetChannelContext *>(context);
    MC_ASSERT(pContext != NULL);

    char szId[50];
    sprintf(szId, "%lld", row->searchId);
    BSTR searchId = MBS2BSTR(szId, false);

    BSTR source = MBS2BSTR(row->source, false);

    BSTR keywords = MBS2BSTR(row->keywords, false);

    // Parse the filter string and create a CMediaSearchFilter object.
    _MediaFilter *pFilter = CreateMediaFilter(row->filters);

    CMediaSearch *search = new CMediaSearch(searchId, source, keywords, pFilter);

    if(pFilter != NULL) {
        pFilter->Release();
    }

    // Append the search object to the search list.
    pContext->searches.push_back(search);

    return 0;
}

} // MobiCast namespace

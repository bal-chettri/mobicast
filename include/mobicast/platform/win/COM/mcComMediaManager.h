/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_WIN_MEDIA_MANAGER_H__
#define __MOBICAST_PLATFORM_WIN_MEDIA_MANAGER_H__

#include <mobicast/platform/win/COM/mcComBase.h>
#include <mobicast/mcDatabase.h>
#include <mcComLib_i.h>
#include <list>

namespace MobiCast
{

extern const IID *CMediaManager_IIDs[];

class CMedia;

/** CMediaManager class. Implements scripting media manager object. */
class CMediaManager : public AutomationComObject<_MediaManager, CMediaManager_IIDs, &IID__MediaManager>
{
public:
    CMediaManager(Database *db);
    virtual ~CMediaManager();

    // _MediaManager methods
    STDMETHODIMP getChannel(BSTR channelId, _Channel **ppRetVal);
    STDMETHODIMP getChannels(VARIANT *pRetVal);
    STDMETHODIMP createSearchFilter(int size, BSTR dateRange, VARIANT_BOOL subtitles,
                                   BSTR type, BSTR quality, int max, _MediaFilter **ppRetVal);
    STDMETHODIMP createSearch(BSTR source, BSTR keywords, IDispatch *pFilter,
                              _MediaSearch **ppRetVal);
    STDMETHODIMP addChannel(BSTR title, VARIANT searches, BSTR *pRetVal);
    STDMETHODIMP listChannel(BSTR channelId, VARIANT *pRetVal);
    STDMETHODIMP deleteChannel(BSTR channelId, VARIANT_BOOL *pRetVal);

    STDMETHODIMP registerMediaHandler(BSTR format, BSTR pluginId);
    STDMETHODIMP listMediaHandlers(VARIANT *pRetVal);

private:
    SAFEARRAY *MediaListToSafeArray(const std::list<CMedia *> &list);
    static _MediaFilter *CreateMediaFilter(const char *szFilter);

    static int GetChannelCallback(const Database::ChannelRow *row, void *context);
    static int GetChannelsCallback(const Database::ChannelRow *row, void *context);
    static int GetChannelSearchesCallback(const Database::ChannelSearchRow *row, void *context);

    Database *_db;
};

} // MobiCast namespace

#endif // !__MOBICAST_PLATFORM_WIN_MEDIA_MANAGER_H__

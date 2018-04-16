/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_WIN_CHANNEL_H__
#define __MOBICAST_PLATFORM_WIN_CHANNEL_H__

#include <mobicast/platform/win/COM/mcComBase.h>
#include <mcComLib_i.h>
#include <list>

namespace MobiCast
{

extern const IID *CChannel_IIDs[];

/** CChannel class. Implements scripting Channel object. */
class CChannel : public AutomationComObject<_Channel, CChannel_IIDs, &IID__Channel>
{
public:
    CChannel(BSTR id, BSTR title);
    CChannel(BSTR id, BSTR title, const std::list<_MediaSearch *> &searches);
    virtual ~CChannel();

    // _MediaSearch methods
    STDMETHODIMP get_id(BSTR *pRetVal);
    STDMETHODIMP get_title(BSTR *pRetVal);
    STDMETHODIMP get_searches(VARIANT *pRetVal);

    // Public internal methods not exposed to COM.
    inline BSTR GetId() const { return _id; }
    inline BSTR GetTitle() const { return _title; }
    inline std::list<_MediaSearch *> GetSearches() const { return _searches; }

private:
    BSTR                        _id;
    BSTR                        _title;
    std::list<_MediaSearch *>   _searches;
};

} // MobiCast namespace

#endif // !__MOBICAST_PLATFORM_WIN_MEDIA_H__

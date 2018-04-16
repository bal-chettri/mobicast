/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_WIN_STORAGE_H__
#define __MOBICAST_PLATFORM_WIN_STORAGE_H__

#include <mobicast/platform/win/COM/mcComBase.h>
#include <mcComLib_i.h>
#include <list>

namespace MobiCast
{

extern const IID *CStorage_IIDs[];

/** CStorage class. Implements scripting storage object. */
class CStorage : public AutomationComObject<_Storage, CStorage_IIDs, &IID__Storage>
{
    struct Volume
    {
        std::string label;
        std::string path;
    };

public:
    CStorage();
    virtual ~CStorage();

    // _Storage methods
    STDMETHODIMP getVolumes(VARIANT *pValue);
    STDMETHODIMP get_mounted(VARIANT_BOOL *ret);
    STDMETHODIMP get_path(BSTR *ret);
    STDMETHODIMP mount(BSTR root, VARIANT_BOOL *ret);
    STDMETHODIMP unmount(VARIANT_BOOL *ret);

private:
    bool IsValidMountRoot(const char *path);
    bool CreateDirectories(const char *base, ...);
    void GetVolumes(std::list<Volume> &listVolumes) const;

    std::string _path;
    bool        _mounted;
    FILE *      _fpindex;
};

} // MobiCast namespace

#endif // !__MOBICAST_PLATFORM_WIN_STORAGE_H__

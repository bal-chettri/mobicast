/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/platform/win/COM/mcComStorage.h>
#include <mobicast/platform/win/COM/mcComUtils.h>
#include <mobicast/mcPathUtils.h>
#include <mobicast/mcDebug.h>
#include <stdarg.h>

namespace MobiCast
{

const IID *CStorage_IIDs[] =
{
    &IID_IUnknown,
    &IID_IDispatch,
    &IID__Storage,
    NULL
};

CStorage::CStorage() :
    _mounted(false),
    _fpindex(NULL)
{ }

CStorage::~CStorage()
{
    VARIANT_BOOL ret = VARIANT_FALSE;
    unmount(&ret);
}

STDMETHODIMP CStorage::getVolumes(VARIANT *pRetVal)
{
    COM_CHECK_PTR(pRetVal)

    std::list<Volume> listVolumes;
    GetVolumes(listVolumes);

    CComArray arrVolumes(listVolumes.size(), false);

    int index = 0;
    for(std::list<Volume>::const_iterator it = listVolumes.begin();
        it != listVolumes.end(); ++it)
    {
        std::string strVol = it->label + std::string(",") + it->path;
        arrVolumes[index].vt = VT_BSTR;
        arrVolumes[index].bstrVal = MBS2BSTR(strVol.c_str(), false);
        ++index;
    }

    pRetVal->vt = VT_ARRAY | VT_VARIANT;
    pRetVal->parray = arrVolumes;

    return S_OK;
}

STDMETHODIMP CStorage::get_mounted(VARIANT_BOOL *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = _mounted ? VARIANT_TRUE : VARIANT_FALSE;
    return S_OK;
}

STDMETHODIMP CStorage::get_path(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    if(_mounted) {
        *pRetVal = MBS2BSTR(_path.c_str(), false);
    } else {
        *pRetVal = NULL;
    }
    return S_OK;
}

STDMETHODIMP CStorage::mount(BSTR root, VARIANT_BOOL *pRetVal)
{
    COM_CHECK_PTR(pRetVal)

    if(!_mounted)
    {
        BSTR2MBS strRoot(root);
        if(
            IsValidMountRoot(strRoot) &&
            CreateDirectories(strRoot, "mobicast", "media", "music", NULL) &&
            CreateDirectories(strRoot, "mobicast", "media", "videos", NULL) &&
            CreateDirectories(strRoot, "mobicast", "media", "images", NULL)
            )
        {
            std::string indexPath(strRoot);
            PathUtils::AppendPathComponent(indexPath, "mobicast\\.index");

            _fpindex = fopen(indexPath.c_str(), "wb");
            if(_fpindex != NULL) {
                _mounted = true;
                _path.assign(strRoot);
            }
        }
    }

    if(!_mounted) {
        MC_LOGD("Failed to mount storage.");
    }

    *pRetVal = _mounted ? VARIANT_TRUE : VARIANT_FALSE;
    return S_OK;
}

STDMETHODIMP CStorage::unmount(VARIANT_BOOL *pRetVal)
{
    COM_CHECK_PTR(pRetVal)

    if(_mounted) {
        fclose(_fpindex);
        _mounted = false;
        _path.clear();
    }

    *pRetVal = !_mounted ? VARIANT_TRUE : VARIANT_FALSE;
    return S_OK;
}

bool CStorage::IsValidMountRoot(const char *path)
{
    return (strlen(path) == 3 && isalpha(path[0]) && path[1] == ':' && path[2] == '\\');
}

bool CStorage::CreateDirectories(const char *base, ...)
{
    std::string strDirPath(base);

    bool ret = true;
    va_list va;
    va_start(va, base);

    const char *dir = va_arg(va, const char *);
    while(dir != NULL)
    {
        PathUtils::AppendPathComponent(strDirPath, dir);
        if(CreateDirectory(strDirPath.c_str(), NULL) == FALSE &&
            GetLastError() != ERROR_ALREADY_EXISTS)
        {
            ret = false;
            break;
        }
        dir = va_arg(va, const char *);
    }

    va_end(va);
    return ret;
}

void CStorage::GetVolumes(std::list<Volume> &listVolumes) const
{
    listVolumes.clear();

    char buffDrives[256];
    GetLogicalDriveStrings(sizeof(buffDrives) / sizeof(buffDrives[0]), buffDrives);

    char *ptrDrive = buffDrives;
    while(*ptrDrive) {
        UINT driveType = GetDriveType(ptrDrive);
        // Add fixed or removable volumes only.
        if(driveType == DRIVE_FIXED || driveType == DRIVE_REMOVABLE) {
            // Get information like lable and flags about this volume.
            char szVolumeName[256] = { 0 };
            DWORD volFlags = 0;
            GetVolumeInformation(ptrDrive, szVolumeName, sizeof(szVolumeName),
                                 NULL, NULL, &volFlags, NULL, 0);
            // Ignore read only volume.
            if(!(volFlags & FILE_READ_ONLY_VOLUME)) {
                Volume volume;
                volume.label = szVolumeName;
                volume.path = ptrDrive;
                listVolumes.push_back(volume);
            }
        }
        ptrDrive+= strlen(ptrDrive) + 1;
    }
}

} // MobiCast namespace

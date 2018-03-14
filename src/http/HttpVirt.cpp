/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include <http/HttpVirt.h>
#include <limits.h>
#include <assert.h>

#ifdef HTTP_PLATFORM_WIN
#   include "Shlwapi.h"
#endif

#ifdef HTTP_PLATFORM_WIN
#   define PATH_DELIM           '\\'
#else
#   define PATH_DELIM           '/'
#endif

#define VIRTUAL_PATH_DELIM      '/'

#ifndef MAX_PATH
#   ifdef PATH_MAX
#       define MAX_PATH PATH_MAX
#   else
#       define MAX_PATH 1024
#   endif
#endif

namespace http
{

VirtualDirectory::VirtualDirectory(const char *baseDir)
{
    // Get canonical path for the base directory.
    char canonicalPath[MAX_PATH];
    assert(GetCanonicalizedPath(baseDir, canonicalPath));    
    _base_path = canonicalPath;

    // Make sure the canonical base path is also an absolute path.
    assert(IsPathAbsolute(_base_path.c_str()));

    // Append a path separator at the end of base path if required.
    if(_base_path.at(_base_path.size() - 1) != PATH_DELIM) {
        char delim[] = { PATH_DELIM, '\0' };
        _base_path.append(delim);
    }   
}

const char *VirtualDirectory::GetBaseDirectory() const
{
    return _base_path.c_str();
}

const char *VirtualDirectory::Resolve(const char *virtPath, std::string &absPath) const
{
    assert(virtPath != NULL);
    
    if(*virtPath == VIRTUAL_PATH_DELIM) {
        ++virtPath;
    }
    
    absPath.assign(_base_path);
    absPath+= virtPath;

    char canonicalPath[MAX_PATH];
    if(GetCanonicalizedPath(absPath.c_str(), canonicalPath))
    {
        absPath = canonicalPath;
        return absPath.c_str();
    }
    return NULL;
}

bool VirtualDirectory::IsPathAllowed(const char *path) const
{
    // Path must begin with base path.
    return (strstr(path, _base_path.c_str()) == path);
}

bool VirtualDirectory::IsPathAbsolute(const char *absPath)
{
    size_t len = strlen(absPath);

    if(len > 1 && (absPath[0] == '\\' || absPath[0] == '/')) {
        return true;
    }
    else if(len > 3 && isascii(absPath[0]) && absPath[1] == ':' && 
            (absPath[2] == '\\' || absPath[2] == '/')) {
        return true;
    }

    return false;
}

bool VirtualDirectory::IsPathRelative(const char *path)
{
    return !IsPathAbsolute(path);
}

char *VirtualDirectory::GetCanonicalizedPath(const char *path, char *buff)
{
#ifdef HTTP_PLATFORM_WIN
    BOOL ret = PathCanonicalizeA(buff, path);    
    return ret == TRUE ? buff : NULL;
#else
    *buff = 0;
    if(realpath(path, buff) || *buff != 0) {
        return buff;
    } else {
        return NULL;
    }
#endif
}

//
// VirtualHost
//
VirtualHost::VirtualHost(const char *doc_root, std::list<std::string> &hostNames, const char *defdoc) :
    _vdir(doc_root),
    _defdoc(defdoc)
{
    // Must have at least one host name.
    assert(!hostNames.empty());
    _hostNames = hostNames;
}

const VirtualDirectory &VirtualHost::GetVirtualDirectory() const
{
    return _vdir;
}

const char *VirtualHost::GetDocumentRoot() const
{
    return _vdir.GetBaseDirectory();
}

const char *VirtualHost::GetHostName() const
{
    return _hostNames.front().c_str();
}

bool VirtualHost::IsValidHostName(const char *hostname) const
{
    for(std::list<std::string>::const_iterator it = _hostNames.begin();
        it != _hostNames.end();
        ++it)
    {
        if(!it->compare(hostname)) {
            return true;
        }
    }
    return false;
}

const char *VirtualHost::GetDefaultDocument() const
{
    return _defdoc.c_str();
}

} // http namespace

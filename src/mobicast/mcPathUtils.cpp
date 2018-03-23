/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/mcPathUtils.h>
#include <mobicast/mcDebug.h>

namespace MobiCast
{

#ifdef MC_PLATFORM_WIN
#   define PATH_DELIM      '\\'
#else
#   define PATH_DELIM      '/'
#endif

std::string PathUtils::GetDirPath(const char *filePath)
{
    MC_ASSERT(filePath != NULL);
    
    const char *ptr = filePath + strlen(filePath);
    while(ptr > filePath && *ptr != '/' && *ptr != '\\') {
        --ptr;
    }

    std::string strDirPath;
    
    if(ptr > filePath)
    {
        strDirPath.assign(filePath, ptr - filePath);
    }

    return strDirPath;
}

std::string PathUtils::MakeUrlPath(const char *path)
{
    MC_ASSERT(path != NULL);

    char *temp = strdup(path);
    MC_ASSERT(temp != NULL);
    
    char *ptr = temp;
    while(*ptr) {
        if(*ptr == '\\') {
            *ptr = '/';
        }
        ++ptr;
    }

    std::string urlPath(temp);
    free(temp);
    
    return urlPath;
}

std::string &PathUtils::AppendPathComponent(std::string &path, const char *component)
{
    if(!path.empty() && path[path.length() - 1] != PATH_DELIM) {
        path+= PATH_DELIM;
    }
    path.append(component);
    return path;
}

} // MobiCast namespace

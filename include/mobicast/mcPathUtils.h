/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PATH_UTILS_H__
#define __MOBICAST_PATH_UTILS_H__

#include <string>

namespace MobiCast
{

class PathUtils
{
public:
    /** Returns the directory path only from file path. */
    static std::string GetDirPath(const char *filePath);

    /** Makes URL friendly path by replacing '\' with '/'. */
    static std::string MakeUrlPath(const char *path);

    /** Appends path component to path. */
    static std::string &AppendPathComponent(std::string &path, const char *component);    
};

} // MobiCast namespace

#endif // !__MOBICAST_PATH_UTILS_H__

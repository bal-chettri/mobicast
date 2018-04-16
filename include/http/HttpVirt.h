/*
 * HttpVirt.h    HTTP web virtual directory.
 *
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __HTTP_VIRT_H__
#define __HTTP_VIRT_H__

#include <http/HttpTypes.h>
#include <string>
#include <list>

namespace http
{

/* VirtualDirectory class. Web server virtual directory object. */
class VirtualDirectory
{
public:
    VirtualDirectory(const char *baseDir);

    /** Returns the canonical base directory path of the virtual directory. */
    const char *GetBaseDirectory() const;

    /** Resolves the virtual path to absolute canonical path. */
    const char *Resolve(const char *virtPath, std::string &absPath) const;

    /** Returns true if the absolute path is allowed for access. */
    bool IsPathAllowed(const char *absPath) const;

private:
    /** Returns true if path is an absolute path. */
    static bool IsPathAbsolute(const char *path);

    /** Returns true if path is a relative path. */
    static bool IsPathRelative(const char *path);

    /** Returns canonicalized path. */
    static char *GetCanonicalizedPath(const char *path, char *buff);

    std::string _base_path;
};

/* VirtualHost class. Web server virtual host configuration. */
class VirtualHost
{
public:
    /** Constructor. Constructs with document root path, list of valid host names 
      * and default document file name. */
    VirtualHost(const char *doc_root, std::list<std::string> &hostNames, const char *defdoc);

    /** Returns the virtual directory object for this virtual host. */
    const VirtualDirectory &GetVirtualDirectory() const;

    /** Returns the document root directory path for this virtual host. */
    const char *GetDocumentRoot() const;

    /** Returns the default host name. */
    const char *GetHostName() const;

    /** Returns true if the host name is valid for this virtual host. */
    bool IsValidHostName(const char *) const;

    /** Returns the default document file name. */
    const char *GetDefaultDocument() const;

private:
    VirtualDirectory _vdir;
    std::list<std::string> _hostNames;
    std::string _defdoc;
};

} // http namespace

#endif /* !__HTTP_VIRT_H__ */

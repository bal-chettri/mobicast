/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLUGIN_H__
#define __MOBICAST_PLUGIN_H__

#include <mobicast/mcTypes.h>
#include <list>
#include <map>

// Plugin capabilities.
#define kPluginCapabilityMediaSource    1<<0
#define kPluginCapabilityPlayer         1<<1

namespace tinyxml2 {
class XMLDocument;
}

namespace MobiCast
{

/** PluginMeta struct. Plugin's meta info. */
struct PluginMeta
{
    std::string id;
    std::string title;
    std::string description;
    std::string author;
    std::string version;
    std::string player;
    std::string main;
    int capabilities;
    bool enabled;
};

/** PluginManager class. */
class PluginManager
{
private:
    /** PluginEntry struct. Entry in the main plugin index XML. */
    struct PluginEntry
    {
        std::string pluginPath;
        bool enabled;
    };

public:
    typedef std::map<std::string, PluginMeta> PluginMap;

    PluginManager();

    /** Load all plugin data. */
    bool LoadPlugins(const char *resPath);

    /** Returns plugin meta info for plugin id. */
    const PluginMeta *FindPlugin(const char *id) const;

    /** Returns the plugin map. */
    inline const PluginMap &GetPlugins() const { return _plugins; }

private:
    bool ReadPluginEntries(tinyxml2::XMLDocument &xml, std::list<PluginEntry> &entries);
    void ReadPluginsMetaData(std::list<PluginEntry> &entries, const char *resPath);

    PluginMap   _plugins;
};

} // MobiCast namespace

#endif // !__MOBICAST_PLUGIN_H__

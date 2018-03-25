/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/mcPlugin.h>
#include <mobicast/mcDebug.h>
#include <mobicast/mcPathUtils.h>
#include <tinyxml2/tinyxml2.h>
#include <list>

#define PLUGINS_INDEX_FILE_NAME    "mcplugins.xml"

namespace MobiCast
{

using namespace tinyxml2;

PluginManager::PluginManager()
{ }

bool PluginManager::LoadPlugins(const char *resPath)
{    
    _plugins.clear();

    // Load main plugins index file.
    std::string pluginsXmlPath(resPath);
    pluginsXmlPath = PathUtils::AppendPathComponent(pluginsXmlPath, PLUGINS_INDEX_FILE_NAME);

    MC_LOGD("Loading plugins index file '%s'.", pluginsXmlPath.c_str());
        
    XMLDocument xml;
    XMLError err = xml.LoadFile(pluginsXmlPath.c_str());
    if(err != XML_SUCCESS || xml.FirstChildElement() == NULL) {
        MC_LOGE("Failed to load plugins index file or invalid XML file.");
        return false;
    }

    // Read list of plugin entries from the plugins index file.
    std::list<PluginEntry> entries;
    if(!ReadPluginEntries(xml, entries)) {
        return false;
    }

    // Read meta info for each plugin in the entry list.
    ReadPluginsMetaData(entries, resPath);

    MC_LOGD("Finished loading plugin meta data. %u plugin(s) found.", _plugins.size());
    return true;
}

const PluginMeta *PluginManager::FindPlugin(const char *id) const
{
    PluginMap::const_iterator it = _plugins.find(std::string(id));
    return it == _plugins.cend() ? NULL : &it->second;
}

bool PluginManager::ReadPluginEntries(tinyxml2::XMLDocument &xml, std::list<PluginEntry> &entries)
{
    XMLElement *elem = xml.FirstChildElement();

    if(!strcmp(elem->Name(), "mcplugins")) 
    {
        elem = elem->FirstChildElement();
        while(elem != NULL) {
            if(!strcmp(elem->Name(), "mcplugin")) {
                PluginEntry entry;
                const char *enabled = elem->Attribute("enabled");
                entry.enabled = (enabled != NULL && !strcmp(enabled, "true"));
                entry.pluginPath = elem->Attribute("path");
                entries.push_back(entry);
            } else {
                MC_LOGE("Invalid element <%s> found in plugins index file.", elem->Name());
                return false;
            }
            elem = elem->NextSiblingElement();
        }
    }
    return true;
}

void PluginManager::ReadPluginsMetaData(std::list<PluginEntry> &entries, const char *resPath)
{
    // All plugins are available at path /res/web/plugins folder.
    std::string strBasePath(resPath);
    strBasePath = PathUtils::AppendPathComponent(strBasePath, "web/plugins");

    // Loop through each plugin entry and load the plugin meta data.
    for(std::list<PluginEntry>::const_iterator it = entries.begin();
        it != entries.end();
        ++it)
    {
        const PluginEntry &entry = *it;

        // Path for plugin's meta data XML file.
        std::string strMetaPath(strBasePath);
        strMetaPath = PathUtils::AppendPathComponent(strMetaPath, entry.pluginPath.c_str());

        MC_LOGD("Loding plugin meta from file '%s'.", strMetaPath.c_str());
        
        // Load plugin meta data XML file.
        XMLDocument xml;
        XMLError err = xml.LoadFile(strMetaPath.c_str());
        if(err != XML_SUCCESS || xml.FirstChildElement() == NULL) {
            MC_LOGE("Failed to load plugin's meta file or invalid XML file.");
            continue;
        }

        // Find root <mcplugin> element.
        XMLElement *elem = xml.FirstChildElement();
        if(strcmp(elem->Name(), "mcplugin")) {
            MC_LOGE("Root element <mcplugin> is missing.");
            continue;
        }

        // Read meta info.
        PluginMeta meta;
        meta.capabilities = 0;
        meta.enabled = entry.enabled;

        elem = elem->FirstChildElement();
        while(elem != NULL)
        {
            if(!strcmp(elem->Name(), "id")) {
                meta.id = elem->GetText();
            } else if(!strcmp(elem->Name(), "title")) {
                meta.title = elem->GetText();
            } else if(!strcmp(elem->Name(), "description")) {
                meta.description = elem->GetText();
            } else if(!strcmp(elem->Name(), "author")) {
                meta.author = elem->GetText();
            } else if(!strcmp(elem->Name(), "version")) {
                meta.version = elem->GetText();
            }
            else if(!strcmp(elem->Name(), "player"))
            {
                // Make relative virtual path to plugin's HTML file.
                meta.player = "plugins";
                PathUtils::AppendPathComponent(meta.player, PathUtils::GetDirPath(entry.pluginPath.c_str()).c_str());
                meta.player = PathUtils::AppendPathComponent(meta.player, elem->GetText());
                meta.player = PathUtils::MakeUrlPath(meta.player.c_str());
            }
            else if(!strcmp(elem->Name(), "main")) 
            {
                // Make relative virtual path to plugin's main JS file.
                meta.main = "plugins";
                PathUtils::AppendPathComponent(meta.main, PathUtils::GetDirPath(entry.pluginPath.c_str()).c_str());
                meta.main = PathUtils::AppendPathComponent(meta.main, elem->GetText());
                meta.main = PathUtils::MakeUrlPath(meta.main.c_str());
            }
            else if(!strcmp(elem->Name(), "capabilities")) 
            {
                XMLElement *elemCap = elem->FirstChildElement();
                while(elemCap != NULL) {
                    if(!strcmp(elemCap->Name(), "capability")) {
                        const char *cap = elemCap->GetText();
                        if(!strcmp(cap, "media-source")) {
                            meta.capabilities|= kPluginCapabilityMediaSource;
                        } else if(!strcmp(cap, "player")) {
                            meta.capabilities|= kPluginCapabilityPlayer;
                        } else {
                            MC_LOGE("Invalid capability <%s> found.", cap);
                        }
                    } else {
                        MC_LOGE("Invalid element <%s> found under <capabilities>.", elem->Name());
                    }
                    elemCap = elemCap->NextSiblingElement();
                }
            } else {
                MC_LOGE("Invalid element <%s> found under <mcplugin>.", elem->Name());
            }
            elem = elem->NextSiblingElement();
        }

        // Add plugin meta to the plugins map.
        _plugins.insert(std::pair<std::string, PluginMeta>(meta.id, meta));
    }
}

} // MobiCast namespace

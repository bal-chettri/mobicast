/*
 * media-api.jss
 *
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
/*
Media management APIs

API: Get plugins
URL: media-apis.jss?cmd=get_plugins

API: Get channels
URL: media-apis.jss?cmd=get_channels

API: Get channel
URL: media-apis.jss?cmd=get_channels&channel_id=id

API: Add channel
URL: media-apis.jss?cmd=add_channel
POST:
{
    "title": "New channel title",
    "searches": [
        {
            "source": "mc.plugin.yt",
            "keywords": [
                "songs",
                "videos"
            ],
            "filters": {
                "size": 1024,
                "date-range": "start-end",
                "subtitles": true,
                "type": "image",
                "quality": "320dpi",
                "max": 100
            }
        }
    ]
}

API: List channel
URL: media-api.jss?cmd=list_channel&channel_id=id
*/

//
// GetPluginsAPI.
//
var GetPluginsAPI = { 
  main: function(_req) {
    var result = {
      'status': 'success',
      'plugins': []
    };

    var plugins = MC.getPlugins();
    var length = plugins.length;
    for(var i = 0; i < length; i++) {
      result.plugins.push(this._pluginMetaObject(plugins[i]));
    }

    _req.replyText(200, JSON.stringify(result), "application/json");
    return true;
  },
  
  _pluginMetaObject: function(pluginItem) {    
    var pm = pluginItem.meta;
    var plugin = pluginItem.plugin;
    
    var pmo = { };    
    pmo['id'] = pm.id;
    pmo['title'] = pm.title;
    pmo['author'] = pm.author;
    pmo['description'] = pm.description;
    pmo['version'] = pm.version;
    pmo['enabled'] = pm.enabled;
    pmo['capabilities'] = [];
    
    var caps = pm.capabilities;
    if(caps & MC.kPluginCapabilityMediaSource) {
      pmo['capabilities'].push('media-source');
    }
    if(caps & MC.kPluginCapabilityPlayer) {
      pmo['capabilities'].push('player');
    }
    if((caps & MC.kPluginCapabilityMediaSource) && plugin != undefined) {
      pmo['filters'] = plugin.getMediaSource().getSearchFilters();
    }
    
    return pmo;
  }
};

//
// GetChannelAPI.
//
var GetChannelAPI = {
  main: function(_req, qs) {
    var channelId = qsGet(qs, 'channel_id');    
    var result;
    
    if(channelId == undefined) {
      result = {
        'status': 'error',
        'message': 'Channel id missing.'
      };
    } else {
      var channel = MC.mm.getChannel(channelId);
      if(channel == undefined) {
        result = {
          'status': 'error',
          'message': 'Invalid channel id.'
        };
      } else {
        result = {
          'status': 'success'
        };
        result['channel'] = this._channelToObject(channel);          
      }
    }
    
    _req.replyText(200, JSON.stringify(result), "application/json");
    return true;
  },
  
  _channelToObject: function (channel) {
      var channelObj = { };
      channelObj['id'] = channel.id;
      channelObj['title'] = channel.title;
      channelObj['searches'] = [];
      var searches = MC.toJsArray(channel.searches);
      for(var i in searches) {
        var search = searches[i];
        
        var searchObj = { };
        searchObj['id'] = search.id; 
        searchObj['source'] = search.mediaSource; 
        searchObj['keywords'] = [];
        var keywordsArr = search.keywords.split(',');
        for(var i in keywordsArr) {
          searchObj['keywords'].push(keywordsArr[i]);
        }
        searchObj['filters'] = this._filterToObject(search.filter);
        
        channelObj['searches'].push(searchObj);
      }
      return channelObj;
  },
  
  _filterToObject: function(filter) {
    var fo = { };
    if(filter != undefined) {
      fo[MC.kPluginMediaSearchFilterType] = filter.size;          
      fo[MC.kPluginMediaSearchFilterDateRange] = filter.dateRange;
      fo[MC.kPluginMediaSearchFilterSubtitles] = filter.subtitles,      
      fo[MC.kPluginMediaSearchFilterQuality] = filter.quality;
      fo[MC.kPluginMediaSearchFilterMax] = filter.max;
    }    
    return fo;
  }
};

//
// GetChannelsAPI.
//
var GetChannelsAPI = {
  main: function(_req) {    
    var result = {
      'status': 'success',
      'channels': []
    };
    
    var channels = MC.toJsArray(MC.mm.getChannels());
    
    for(var i in channels) {
      var channel = channels[i];
      var channelObj = { };
      channelObj['id'] = channel.id;
      channelObj['title'] = channel.title;
      result.channels.push(channelObj);
    }

    _req.replyText(200, JSON.stringify(result), "application/json");
    return true;
  }
};

//
// AddChannelAPI.
//
var AddChannelAPI = {
  main: function(_req) {
    var channel = JSON.parse(_req.body);

    if(!this._validateChannel(channel)) {
      _req.replyText(200, JSON.stringify(this._errorResult('Invalid channel JSON')), "application/json");
      return;
    }
    
    var searches = [];
    
    for(var i in channel.searches)
    {
      var search = channel.searches[i];
      
      // Create a search filter object.
      var size = 0;
      var dateRange = null;
      var subtitles = false;
      var type = null;
      var quality = null;
      var max = 0;
      
      if('size' in search.filters) {
        size = search.filters['size'];
      }
      if('date-range' in search.filters) {
        dateRange = search.filters['date-range'];
      }
      if('subtitles' in search.filters) {
        subtitles = search.filters['subtitles'];
      }
      if('type' in search.filters) {
        type = search.filters['type'];
      }
      if('quality' in search.filters) {
        quality = search.filters['quality'];
      }
      if('max' in search.filters) {
        max = search.filters['max'];
      }
      
      var filterObj = MC.mm.createSearchFilter(size, dateRange, subtitles, type, quality, max);
      MC.log('AddChannelAPI', 'filterObj = ' + typeof filterObj);
      
      // Create search object.
      var searchObj = MC.mm.createSearch(search.source, search.keywords.join(','), filterObj);
      
      // Append search object to array.
      searches.push(searchObj);
    }
    
    // Convert to native array.
    searches = MC.toNativeArray(searches);
    
    // Add the channel.
    var channelId = MC.mm.addChannel(channel.title, searches);

    var result = {
      'status': 'success',
      'channel': {
        'id': channelId,
        'title': channel.title
      }
    };

    _req.replyText(200, JSON.stringify(result), "application/json");
    return true;
  },
  
  _validateChannel: function(channel) {
    if(!OBJECT.isObject(channel)) {
      MC.log('AddChannelAPI', 'Root is not an object');
      return false;
    }

    if(!OBJECT.has(channel, 'title', 'string') ||
       !OBJECT.has(channel, 'searches', 'array')
       )
    {
      MC.log('AddChannelAPI', 'Invalid title searches');
      return false;
    }

    for(var i in channel.searches) {
      var search = channel.searches[i];
      if(!OBJECT.isObject(search)) {
        MC.log('AddChannelAPI', 'search not an object');
        return false;
      }
      if(!OBJECT.has(search, 'source', 'string') ||
         !OBJECT.has(search, 'keywords', 'array') ||
         !OBJECT.has(search, 'filters', 'object')
         )
      {
        MC.log('AddChannelAPI', 'Invalid source keywords filters');
        return false;
      }
      for(var i in search.keywords) {
        if(typeof search.keywords[i] != 'string') {
          MC.log('AddChannelAPI', 'Invalid keyword');
          return false;
        }
      }
      for(var key in search.filters) {
        if(typeof key != 'string') {
          MC.log('AddChannelAPI', 'Invalid filter key');
          return false;
        }
        if(key != 'size' &&
           key != 'date-range' &&
           key != 'subtitles' &&
           key != 'type' &&
           key != 'quality' &&
           key != 'max')
        {
          MC.log('AddChannelAPI', 'Invalid filter name');
          return false;
        }
        var type = typeof search.filters[key];
        if((key == 'size' && type == 'number') ||
           (key =='date-range' && type == 'string') ||
           (key == 'subtitles' && type == 'boolean') ||
           (key == 'type' && type == 'string') ||
           (key == 'quality' && type == 'string') ||
           (key == 'max' && type == 'number')
           )
        { }
        else {
          MC.log('AddChannelAPI', 'Invalid filter value: ' + key);
          return false;
        }
      }
    }
    return true;
  },

  _errorResult: function(msg) {
    return {
      'status': 'error',
      'message': msg
      };
  }
};

//
// ListChannelAPI.
//
var ListChannelAPI = {
  main: function(_req, qs) {
    var channelId = qsGet(qs, 'channel_id');
    
    if(channelId == undefined) {
      _req.replyText(200, JSON.stringify(this._errorResult('Invalid channel JSON.')), "application/json");
      return;
    }
        
    var result;
    var items = MC.mm.listChannel(channelId);    
    
    if(items == undefined) {
      result = this._errorResult('No media items.')
    } else {
      var result = {
        'status': 'success',
        'channel': {
          'id': channelId
        },
        'items': []
      };
      items = MC.toJsArray(items);
      
      for(var i in items) {
        result.items.push(this._MediaToObject(items[i]));
      }
    }
    
    _req.replyText(200, JSON.stringify(result), "application/json");
    return true;
  },
  
  _MediaToObject: function(media) {
    var media = {
      'type': media.type,
      'title': media.title,
      'mediaUrl': media.mediaUrl,
      'thumbnailUrl': media.thumbnailUrl,
      'duration': media.duration,
      'format': media.format
    };
    return media;
  },
  
  _errorResult: function(msg) {
    return {
      'status': 'error',
      'message': msg
      };
  }
};

// Register service.
MC.registerService("/services/media-api.jss", function(_req, _resp) {
  try {
    MC.log('media-api.jss', 'Service invoked qs = ' + _req.rawQueryString);

    var qs = parseQueryString(_req.rawQueryString);
    var cmd = qsGet(qs, 'cmd');

    if(cmd == 'get_plugins')
    {
      return GetPluginsAPI.main(_req);
    }
    else if(cmd == 'get_channel')
    {
      return GetChannelAPI.main(_req, qs);
    }
    else if(cmd == 'get_channels')
    {
      return GetChannelsAPI.main(_req);
    }
    else if(cmd == 'add_channel')
    {
      return AddChannelAPI.main(_req);
    }
    else if(cmd == 'list_channel')
    {
      return ListChannelAPI.main(_req, qs);
    }       
    else
    {
      var result = {
        'status': 'error',
        'message': 'Invalid command.'
      };
      _req.replyText(200, JSON.stringify(result), "application/json");
      return true;
    }
  }
  catch(e) {
    var result = {
      'status': 'error',
      'message': 'Internal error occurred: ' + e.message + ', Line no: ' + e.lineNumber
    };
    _req.replyText(200, JSON.stringify(result), "application/json");
    return true;
  }
});

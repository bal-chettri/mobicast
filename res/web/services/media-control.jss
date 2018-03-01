/*
 * media-control.jss
 *
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
/*
Media control APIs

API: Play media
URL: media-control.jss?cmd=play_media
POST:
{
    "mediaUrl": "content-url",
    "format": "media-format",
    "player": "plugin-id",
    "registerPlayer": boolean
}

API: Pause media
URL: media-control.jss?cmd=pause_media

API: Rewind media
URL: media-control.jss?cmd=rewind_media

API: Volume control
URL: media-control.jss?cmd=vol_up
URL: media-control.jss?cmd=vol_down
URL: media-control.jss?cmd=vol_mute
URL: media-control.jss?cmd=vol_unmute
*/

//
// PlayMediaAPI.
//
var PlayMediaAPI = {
  main: function(_req, qs) {
      var media = JSON.parse(_req.body);
      if(!this._validateMedia(media)) {
        _req.replyText(200, this._errorResult('Invalid media JSON.'), "application/json");
      } else {
        var pid = media['player'];
        var pinfo = MC.findPlugin(pid);
        if(pinfo == null) {
          _req.replyText(200, this._errorResult('Plugin not found.'), "application/json");
        } else if(pinfo.plugin == undefined) {
          _req.replyText(200, this._errorResult('Plugin not registered.'), "application/json");
        } else {
          var capabilities = pinfo.meta.capabilities;
          if(!(capabilities & MC.kPluginCapabilityPlayer)) {
            _req.replyText(200, this._errorResult('Plugin doesn\'t support playback.'), "application/json");
          } else {
            var format = media['format'];
            var formats = pinfo.plugin.getPlayer().getMediaFormats();
            if(formats.indexOf(format) < 0) {
              _req.replyText(200, this._errorResult('Plugin doesn\'t support the format.'), "application/json");
            } else if(!MC.activatePlugin(pid)) {
              _req.replyText(200, this._errorResult('Failed to activate the plugin.'), "application/json");
            } else {
              var url = media['mediaUrl'];
              if(pinfo.plugin.getPlayer().play(url)) {
                var result = {
                  'status': 'success'
                }
                if('registerPlayer' in media && media['registerPlayer'] == true) {
                  MC.mm.registerMediaHandler(format, pid);
                }
                _req.replyText(200, JSON.stringify(result), "application/json");
              } else {
                _req.replyText(200, this._errorResult('Failed to play media.'), "application/json");
              }
            }
          }
        }
      }
      return true;
    },
    
    _validateMedia: function(media) {
      if(!OBJECT.isObject(media)) {
        return false;
      }
      if(!OBJECT.has(media, 'mediaUrl', 'string')) {
        return false;
      }
      if(!OBJECT.has(media, 'format', 'string')) {
        return false;
      }
      if(!OBJECT.has(media, 'player', 'string')) {
        return false;
      }
      if('registerPlayer' in media) {
        if(!OBJECT.has(media, 'registerPlayer', 'boolean')) {
          return false;
        }
      }
      return true;
    },
    
    _errorResult: function(msg) {
      return JSON.stringify({
        'status': 'error',
        'message': msg
        });
    }
};

//
// PauseMedia API.
//
var PauseMediaAPI = {
  main: function(_req, qs, pause) {
    var pinfo = MC.getActivePlugin();
    if(pinfo == null) {
      _req.replyText(200, this._errorResult('No active plugin.'), "application/json");
    } else {
      var capabilities = pinfo.meta.capabilities;
      if(!(capabilities & MC.kPluginCapabilityPlayer)) {
        _req.replyText(200, this._errorResult('Plugin doesn\'t support playback.'), "application/json");
      } else {
        var player = pinfo.plugin.getPlayer();
        if(pause) {
          player.pause();
        } else {
          player.resume();
        }
        var result = {
          'status': 'success'
        }
        _req.replyText(200, JSON.stringify(result), "application/json");
      }
    }
    return true;
  },
  
  _errorResult: function(msg) {
    return JSON.stringify({
      'status': 'error',
      'message': msg
    });
  }
};

//
// RewindMediaAPI.
//
var RewindMediaAPI = {
  main: function(_req, qs) {
    var pinfo = MC.getActivePlugin();
    if(pinfo == null) {
      _req.replyText(200, this._errorResult('No active plugin.'), "application/json");
    } else {
      var capabilities = pinfo.meta.capabilities;
      if(!(capabilities & MC.kPluginCapabilityPlayer)) {
        _req.replyText(200, this._errorResult('Plugin doesn\'t support playback.'), "application/json");
      } else {        
        pinfo.plugin.getPlayer().rewind();
        var result = {
          'status': 'success'
        }
        _req.replyText(200, JSON.stringify(result), "application/json");
      }
    }
    return true;
  },
  
  _errorResult: function(msg) {
    return JSON.stringify({
      'status': 'error',
      'message': msg
    });
  }
};

//
// Volume API.
//
// Following commands are handled:
// cmd=vol_up
// cmd=vol_down
// cmd=vol_mute
// cmd=vol_unmute
//
var VolumeAPI = {
  main: function(_req, qs, cmd) {
    var pinfo = MC.getActivePlugin();
    if(pinfo == null) {
      _req.replyText(200, this._errorResult('No active plugin.'), "application/json");
    } else {
      var capabilities = pinfo.meta.capabilities;
      if(!(capabilities & MC.kPluginCapabilityPlayer)) {
        _req.replyText(200, this._errorResult('Plugin doesn\'t support playback.'), "application/json");
      } else {
        var player = pinfo.plugin.getPlayer();
        if(cmd == 'vol_up') {
          player.stepVolume(1);
        } else if(cmd == 'vol_down') {
          player.stepVolume(-1);
        } else if(cmd == 'vol_mute') {
          player.muteVolume();
        } else if(cmd == 'vol_unmute') {
          player.unmuteVolume();
        } else  {
          _req.replyText(200, this._errorResult('Invalid command.'), "application/json");
        }
        var result = {
          'status': 'success'
        }
        _req.replyText(200, JSON.stringify(result), "application/json");
      }
    }
    return true;
  },
  
  _errorResult: function(msg) {
    return JSON.stringify({
      'status': 'error',
      'message': msg
    });
  }
};

// Register service
MC.registerService("/services/media-control.jss", function(_req, _resp) {
  try {
    MC.log('media-control.jss', 'Service invoked qs = ' + _req.rawQueryString);

    var qs = parseQueryString(_req.rawQueryString);
    var cmd = qsGet(qs, 'cmd');

    if(cmd == 'play_media')
    {
      return PlayMediaAPI.main(_req, qs);
    }
    else if(cmd == 'pause_media')
    {
      return PauseMediaAPI.main(_req, qs, true);
    }
    else if(cmd == 'resume_media')
    {
      return PauseMediaAPI.main(_req, qs, false);
    }
    else if(cmd == 'rewind_media')
    {
      return RewindMediaAPI.main(_req, qs);
    }
    else if(cmd == 'vol_up' || 
            cmd == 'vol_down' || 
            cmd == 'vol_mute' || 
            cmd == 'vol_unmute')
    {
      return VolumeAPI.main(_req, qs, cmd);
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

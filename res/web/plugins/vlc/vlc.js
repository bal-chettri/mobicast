/*
 * vlc.js
 *
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
// vlc plugin object
var _vlc = { }

// Plugin player interface

_vlc.player = { 
  mediaURL: undefined,
  ready: false
};

_vlc.player.getMediaFormats = function() {
  return [
    '.3gp',
    '.avi',
    '.flv',
    '.mkv',
    '.mov',
    '.m4v',
    '.mp4',
    '.mpg',
    
    '.mp3',
    '.ogg',
    
    '.youtube'
  ];
}

_vlc.player.play = function(url) {
  this.mediaURL = url;
  MC.log('VLC', 'Loading media ' + this.mediaURL);
  if(this.ready) {
    this._play();
  }
  return true;
}

_vlc.player.pause = function() {
  if(this.ready) {
    MC.log('VLC', 'Pausing  media');
    var playlist = document.getElementById("vlc").playlist;
    playlist.pause();
  }
}

_vlc.player.resume = function() {
  if(this.ready) {
    MC.log('VLC', 'Resuming media');
    var playlist = document.getElementById("vlc").playlist;
    playlist.play();
  }
}

_vlc.player.rewind = function() {
  if(this.ready) {
    MC.log('VLC', 'Rewinding video');
    var vlc = document.getElementById("vlc");
    vlc.input.time = 0;
  }
}

_vlc.player.stop = function() {
  if(this.ready) {
    MC.log('VLC', 'Stopping media');
    var playlist = document.getElementById("vlc").playlist;
    playlist.stop();
  }
}

_vlc.player.stepVolume = function(step) {
  if(this.ready) {
    var vlc = document.getElementById('vlc');    
    var kScale = 20;    
    var vol = (vlc.audio.volume + 0.0) / kScale;
    vol = Math.round(vol + step);
    vol = (vol < 0 ? 0 : vol > 10 ? 10 : vol) * kScale;
    MC.log('VLC', 'Stepping volume by ' + step + ', New volume = ' + vol);    
    vlc.audio.volume = vol;
    vlc.audio.mute = false;
  }
}
_vlc.player.muteVolume = function() {
  if(this.ready) {
    MC.log('VLC', 'Muting volume');
    var vlc = document.getElementById('vlc');
    vlc.audio.mute = true;
  }
}
_vlc.player.unmuteVolume = function() {
  if(this.ready) {
    MC.log('VLC', 'Unmuting volume');
    var vlc = document.getElementById('vlc');
    vlc.audio.mute = false;
  }
}

// Player internal methods

_vlc.player._play = function() {
  if(this.ready && this.mediaURL != undefined) {
    MC.log('VLC', 'Playing media ' + this.mediaURL);
    var playlist = document.getElementById("vlc").playlist;
    playlist.clear();
    playlist.add(this.mediaURL, "item0", ":no-video-title-show");    
    playlist.play();
    this.mediaURL = undefined;
  }
}

_vlc.player._isReady = function() {
  if(this.ready == false) {
    try {
      var playlist = document.getElementById("vlc").playlist;
      if(playlist != undefined && playlist != null) {
        this.ready = true;
      }
    } catch(e) { }
  }
  return this.ready;
}

_vlc.player._close = function() {  
  this.mediaURL = undefined;
  this.ready = false;
}

// Plugin interface

_vlc.onLoad = function() {
  MC.log('VLC', 'VLC plugin loaded');
  // Wait until player is ready.
  _vlc.readyTimer = window.setInterval(function() {
    MC.log('VLC', 'Checking if player is ready...');
    if(_vlc.player._isReady()) {
      MC.log('VLC', 'Player is ready.');
      window.clearInterval(_vlc.readyTimer);
      _vlc.readyTimer = undefined;
      // Play loaded item, if any.
      _vlc.player._play();
    }
  }, 2000);
}

_vlc.onClose = function() {
  MC.log('VLC', 'VLC plugin unloaded');
  if(_vlc.readyTimer != undefined) {
    window.clearInterval(_vlc.readyTimer);
    _vlc.readyTimer = undefined;
  }
  this.player._close();
}

_vlc.getPlayer = function() {
  return _vlc.player;
}

// Register vlc plugin
MC.registerPlugin('mc.plugin.vlc', _vlc);

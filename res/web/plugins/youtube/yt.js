/*
 * yt.js
 *
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
var _ytBase = _ytBase || { ready: false, player: null };
var onYouTubeIframeAPIReady = onYouTubeIframeAPIReady || function() {
  _ytBase.ready = true;
  _ytBase.player._play();
}

// Youtube plugin object
var _yt = { }

// Plugin player interface

_yt.player = {  
  mediaId: null,
  _player: null
};

_yt.player.getMediaFormats = function() {
  return ['.youtube'];
}

_yt.player.play = function(url) {
  /*
   Parse video ID from YouTube video URL.
   http://web.archive.org/web/20160926134334/http://lasnv.net/foro/839/Javascript_parsear_URL_de_YouTube
   https://stackoverflow.com/questions/3452546/how-do-i-get-the-youtube-video-id-from-a-url
   */
  var regExp = /^.*((youtu.be\/)|(v\/)|(\/u\/\w\/)|(embed\/)|(watch\?))\??v?=?([^#\&\?]*).*/;
  var match = url.match(regExp);
  var mediaId = (match && match.length >= 8 && match[7].length == 11) ? match[7] : false;
  if(mediaId == false) {
    MC.loge('YT', 'Failed to parse media ID from URL ' + url);
    return false;
  } else {
    this.mediaId = mediaId;
    MC.log('YT', 'Queueing media ' + this.mediaId);
    if(_ytBase.ready) {
      this._play();
    }
    return true;
  }
}

_yt.player.pause = function() {
  if(this._player != null) {
    MC.log('YT', 'Pausing  media');
    this._player.pauseVideo();
  }
}
_yt.player.resume = function() {
  if(this._player != null) {
    MC.log('YT', 'Resuming  media');
    this._player.playVideo();
  }
}
_yt.player.rewind = function() {
  if(this._player != null) {
    MC.log('YT', 'Rewinding media');
    this._player.seekTo(0, true);
  }
}
_yt.player.stop = function() {
  if(this._player != null) {
    MC.log('YT', 'Stopping media');
    this._player.stopVideo();
  }
}

_yt.player.stepVolume = function(step) {
  if(this._player != null) {
    var kScale = 10;
    var vol = (this._player.getVolume() + 0.0) / kScale;
    vol = Math.round(vol + step);
    vol = (vol < 0 ? 0 : vol > 10 ? 10 : vol) * kScale;
    MC.log('YT', 'Stepping volume by ' + step + ', New volume = ' + vol);
    this._player.setVolume(vol);
    this._player.unMute();
  }
}
_yt.player.muteVolume = function() {
  if(this._player != null) {
    MC.log('YT', 'Muting volume');
    this._player.mute();
  }
}
_yt.player.unmuteVolume = function() {
  if(this._player != null) {
    MC.log('YT', 'Unmuting volume');
    this._player.unMute();
  }
}

// Internal player methods.

_yt.player._destroy = function() {
  MC.log('YT', 'Destroying player IFrame');
  if(this._player != null) {
    this._player.destroy();
    this._player = null;
  }
  this.mediaId = null;
}

_yt.player._play = function() {
  if(_ytBase.ready && this.mediaId != null) {    
    if(this._player == null) {
      MC.log('YT', 'Creating player IFrame with media ' + this.mediaId);
      var playerConfig = {
        height: '100%',
        width: '100%',
        videoId: this.mediaId,
        playerVars: {
          'enablejsapi': 1,
          'autoplay': 0,
          'controls': 0,
          'fs': 0,
          'loop': 0,
          'showinfo': 0,
          'iv_load_policy': 3,
          'disablekb': 0,
          'rel': 0,
          'modestbranding': 1
        },
        events: {
          'onReady': this._onYouTubePlayerReady,
          'onStateChange': this._onYouTubePlayerStateChange
        }
      };
      this._player = new YT.Player('_ytplayer', playerConfig);
    } else {
      MC.log('YT', 'Loading media ' + this.mediaId);
      this._player.loadVideoById(this.mediaId);
    }
  }
}

// Youtube player event handlers.

_yt.player._onYouTubePlayerReady = function(event) {
  MC.log('YT', 'Player is ready, playing media');
  event.target.playVideo();
}

_yt.player._onYouTubePlayerStateChange = function(event) {
  if(event.data == YT.PlayerState.ENDED) {
    MC.log('YT', 'Player state = ENDED');
  }
}

// Plugin interface

_yt.preLoad = function() {
  MC.log('YT', 'Youtube plugin loading');
  _ytBase.player = this.player;
}

_yt.onLoad = function() {
  MC.log('YT', 'Youtube plugin loaded');
}

_yt.onClose = function() {
  MC.log('YT', 'Youtube plugin unloaded');
  this.player._destroy();
  _ytBase.player = null;
}

_yt.getMediaSource = function() {
  return this.mediaSource;
}

_yt.getPlayer = function() {
  return this.player;
}

// Register youtube plugin
MC.registerPlugin('mc.plugin.yt', _yt);

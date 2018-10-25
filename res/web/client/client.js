/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
"use strict";

var ApiFactory =
{
  host: window.location.hostname,
  port: window.location.port,

  _url: function(api)
  {
    let surl = 'http://' + this.host;
    if(this.port != 80) {
      surl+= ':' + this.port;
    }
    surl+= api;
    return surl;
  },

  getPluginsUrl: function() {
    return this._url('/services/media-api.jss?cmd=get_plugins');
  },
  listMediaHandlersUrl: function() {
    return this._url('/services/media-api.jss?cmd=list_media_handlers');
  },

  getChannelsUrl: function() {
    return this._url('/services/media-api.jss?cmd=get_channels');
  },
  getChannelUrl: function(id) {
    return this._url('/services/media-api.jss?cmd=get_channel&channel_id=' + id);
  },
  addChannelUrl:function() {
    return this._url('/services/media-api.jss?cmd=add_channel');
  },
  listChannelUrl: function(id) {
    return this._url('/services/media-api.jss?cmd=list_channel&channel_id=' + id);
  },
  deleteChannelUrl: function(id) {
    return this._url('/services/media-api.jss?cmd=delete_channel&channel_id=' + id);
  },

  playMediaUrl: function() {
    return this._url('/services/media-control.jss?cmd=play_media');
  },
  pauseMediaUrl: function() {
    return this._url('/services/media-control.jss?cmd=pause_media');
  },
  resumeMediaUrl: function() {
    return this._url('/services/media-control.jss?cmd=resume_media');
  },
  rewindMediaUrl: function() {
    return this._url('/services/media-control.jss?cmd=rewind_media');
  },

  muteMediaUrl: function() {
    return this._url('/services/media-control.jss?cmd=vol_mute');
  },
  unmuteMediaUrl: function() {
    return this._url('/services/media-control.jss?cmd=vol_unmute');
  },
  volStepUpUrl: function() {
    return this._url('/services/media-control.jss?cmd=vol_up');
  },
  volStepDownUrl: function() {
    return this._url('/services/media-control.jss?cmd=vol_down');
  }
};

class SimpleRequestBroker
{
  static get(apiUrl, callback)
  {
    return $.get(apiUrl, (data, textStatus, jqxhr) => {
      if(textStatus === 'success' && data.status === 'success') {
        callback(data);
      }
    });
  }
  static post(apiUrl, data, callback)
  {
    return $.post(apiUrl, data, (data, textStatus, jqxhr) => {
      if(textStatus === 'success' && data.status === 'success') {
        callback(data);
      }
    });
  }
}

class DataRequestBroker
{
  static get(apiUrl, callback)
  {
    return $.get(apiUrl, (data, textStatus, jqxhr) => {
      callback(data, textStatus === 'success' && data.status === 'success');
    });
  }
  static post(apiUrl, data, callback)
  {
    return $.post(apiUrl, data, (data, textStatus, jqxhr) => {
      callback(data, textStatus === 'success' && data.status === 'success');
    });
  }
}

/**  Global app state data. */
var AppState = { };

/** PlayerController class. */

const kPlayerControllerStateReady = 0;
const kPlayerControllerStatePlaying = 1;
const kPlayerControllerStatePaused = 2;

class PlayerController
{
  constructor()
  {
    this.curMediaRequest = undefined;
    this.playerState = kPlayerControllerStateReady;
    this.playerEventListeners = [];
    this.muted = false;
  }

  static get()
  {
    if(PlayerController.instance == undefined) {
      PlayerController.instance = new PlayerController();
    }
    return PlayerController.instance;
  }

  static destroy()
  {
    PlayerController.instance = null;
  }

  addEventListener(listener)
  {
    this.playerEventListeners.push(listener);
  }

  removeEventListener(listener)
  {
    let index = this.playerEventListeners.indexOf(listener);
    if(index != -1) {
      this.playerEventListeners.splice(index, 1);
    }
  }

  getCurrentMediaRequest()
  {
    return this.curMediaRequest;
  }

  getPlayerState()
  {
    return this.playerState;
  }

  isMuted()
  {
    return this.muted;
  }

  play(mediaRequest)
  {
    let medaInfo = mediaRequest.mediaInfo;
    let mediaInfoJson = JSON.stringify(medaInfo);
    SimpleRequestBroker.post(ApiFactory.playMediaUrl(), mediaInfoJson, (data) => {
      this.curMediaRequest = mediaRequest;
      if(medaInfo.registerPlayer) {
        AppState.mediaHandlers[medaInfo.format] = medaInfo.player;
      }
      this.setPlayerState(kPlayerControllerStatePlaying);
    }).always(() => {
      ;
    });
  }

  pause()
  {
    if(this.playerState === kPlayerControllerStatePlaying) {
      SimpleRequestBroker.get(ApiFactory.pauseMediaUrl(), (data) => {
        this.setPlayerState(kPlayerControllerStatePaused);
      }).always(() => {
        ;
      });
    }
  }

  resume()
  {
    if(this.playerState === kPlayerControllerStatePaused) {
      SimpleRequestBroker.get(ApiFactory.resumeMediaUrl(), (data) => {
        this.setPlayerState(kPlayerControllerStatePlaying);
      }).always(() => {
        ;
      });
    }
  }

  rewind()
  {
    if(this.playerState === kPlayerControllerStatePlaying ||
       this.playerState === kPlayerControllerStatePaused)
    {
      SimpleRequestBroker.get(ApiFactory.rewindMediaUrl(), (data) => {
        // Rewinding doesn't play automatically.
        // this.setPlayerState(kPlayerControllerStatePlaying);
      }).always(() => {
        ;
      });
    }
  }

  toggleMute()
  {
    if(this.playerState === kPlayerControllerStatePlaying ||
       this.playerState === kPlayerControllerStatePaused)
    {
      let apiUrl = this.muted ? ApiFactory.unmuteMediaUrl() : ApiFactory.muteMediaUrl();
      SimpleRequestBroker.get(apiUrl, (data) => {
        this.toggleMuteState();      
      }).always(() => {
        ;
      });
    }
  }

  stepVolume(up)
  {
    let apiUrl = up ? ApiFactory.volStepUpUrl() : ApiFactory.volStepDownUrl();
    SimpleRequestBroker.get(apiUrl, (data) => {
      if(this.muted) {
        this.toggleMuteState();
      }
    }).always(() => {
      ;
    });
  }

  setPlayerState(newState)
  {
    if(newState !== this.playerState) {
      let oldState = this.playerState;
      this.playerState = newState;
      this.onPlayerStateChanged(oldState, this.playerState);
    }
  }  

  toggleMuteState()
  {
    this.muted = !this.muted;
    this.onMuteStateChanged(this.muted);
  }

  onPlayerStateChanged(oldState, newState)
  {
    this.playerEventListeners.forEach((handler, index) => {
      handler.onPlayerStateChanged(oldState, newState);
    });
  }

  onMuteStateChanged(muteState)
  {
    this.playerEventListeners.forEach((handler, index) => {
      handler.onMuteStateChanged(muteState);
    });
  }
}

/** View class. Base abstract view class. */
class View
{
  constructor(title)
  {
    this.title = title;
    this.contentRoot = $('.wrapper #content #main-content');
  }

  /** Returns the title of the view. */
  getTitle()
  {
    return this.title;
  }

  /** Returns the root of content. */
  getContentRoot()
  {
    return this.contentRoot;
  }

  /** Refresh the view's content. */
  refresh()
  {
    throw "Not implemented";
  }
  
  /** Redraws the view's content. */
  repaint()
  {
    throw "Not implemented";
  }

  /** View is shown on the screen. */
  onShow() {}

  /** View has finished rendering its content. */
  onRender() {}

  /** View is no closed and no longer shown on the screen. */
  onClose() {}

  /** View is being destroyed. */
  onDestroy() {}

  showLoader()
  {
    $('.loader').show();
  }

  hideLoader()
  {
    $('.loader').hide();
  }

  setContent(html)
  {
    $('#main-content').html(html);
  }
}

class Dialog
{
  constructor(config)
  {
    this.config = config;

    $('.overlay').on('click', () => {
      this.hide();
    });

    this.getItem('btn-close').on('click', () => {
      this.hide();
    });
  }

  getTemplate()
  {
    return this.config.template;
  }

  getNode()
  {
    return $('#' + this.getTemplate());
  }

  getItem(id)
  {
    return $(this.getNode().find('#' + id)[0]);
  }

  show()
  {
    this.onShow();
    $('.overlay').fadeIn();
    this.getNode().css('display', 'flex');
  }

  hide()
  {
    $('.overlay').fadeOut();
    this.getNode().css('display', 'none');
    this.onHide();
  }

  onShow() {}

  onHide() {}
}

class PlayerDialog extends Dialog
{
  constructor()
  {
    super({
      'template': 'player'
      });

      console.log('PlayerDialog constructed...');

      this.playerEventListener = {
        onPlayerStateChanged: (oldState, newState) => {
          this._updateUI();
        },

        onMuteStateChanged: (muteState) => {
          this._updateUI();
        }
      };

      PlayerController.get().addEventListener(this.playerEventListener);

      this.getItem('btn-pause').on('click', function() {
        let pc = PlayerController.get();
        if(pc.getPlayerState() === kPlayerControllerStatePaused) {
          pc.resume();
        } else {
          pc.pause();
        }
      });

      this.getItem('btn-rewind').on('click', function() {
        PlayerController.get().rewind();
      });

      this.getItem('btn-mute').on('click', function() {
        PlayerController.get().toggleMute();
      });

      this.getItem('btn-vol-down').on('click', function() {
        PlayerController.get().stepVolume(false);
      });

      this.getItem('btn-vol-up').on('click', function() {
        PlayerController.get().stepVolume(true);
      });
  }

  onShow()
  {
    super.onShow();
    this._updateUI();
  }

  _updateUI()
  {
    let pc = PlayerController.get(), playerState = pc.getPlayerState();

    let currentMedia = pc.getCurrentMediaRequest();
    let caption = this.getItem('caption');

    if(currentMedia == undefined || currentMedia == null) {
      caption.html('Try playing something...');
    } else {
      caption.html(pc.getCurrentMediaRequest().title);
    }

    this._updateButtonUI();
  }

  _updateButtonUI()
  {
    let pc = PlayerController.get();

    let pauseBtn = this.getItem('btn-pause');
    pauseBtn.html(pc.getPlayerState() === kPlayerControllerStatePaused ? 'Resume' : 'Pause');

    let muteBtn = this.getItem('btn-mute');
    muteBtn.html(pc.isMuted() ? 'Unmute' : 'Mute');
  }
}

const kChannelEditModeCreate = 0;
const kChannelEditModeEdit = 1;

class ChannelDialog extends Dialog
{
  constructor(editMode, data)
  {
    super({
      'template': 'channel-dialog'
    });

    this.kMaxSearches = 3;

    this.editMode = editMode;
    this.mediaSources = this._getMediaSources();
    this.eventHandler = null;
    this.regexTitle = /^([a-z]|[A-Z])+(([a-z]|[A-Z]|[0-9]| )*([a-z]|[A-Z]|[0-9]))*$/
    this.regexKeyword = /^([a-z]|[A-Z])+(([a-z]|[A-Z]|[0-9]| )*([a-z]|[A-Z]|[0-9]))*$/

    if(this.editMode === kChannelEditModeCreate) {
      this.data = {
        id: '',
        title: '',
        searches: [
          {
            source: '',
            keywords: [],
            filters: {}
          }
        ]
      };
    } else {
      this.data = data;
    }

    this.curSearch = this.data.searches.length > 0 ? 0 : -1;

    this._init();
    this._advise();
  }

  dispose()
  {
    this._unadvice();
    this.eventHandler = undefined;
  }

  setEventHandler(handler)
  {
    this.eventHandler = handler;
    return this;
  }

  getData()
  {
    return this.data;
  }

  _init()
  {
    this._updateUI();
    this._updateSearchUI();

    $(this.getNode().find('.title')[0]).html(
      this.editMode === kChannelEditModeCreate ? 'Add Channel' : 'Edit Channel'
      );

    this.getItem('btn-create').html(
      this.editMode === kChannelEditModeCreate ? 'Add' : 'Update'
      );
  }

  _advise()
  {
    let self = this;

    this.getItem('channel_title').change(function() {
      let title = $(this).val();
      if(title.match(self.regexTitle) != null) {
        self.data.title = title;
      } else {
        $(this).val(self.data.title);
      }
    });

    $(this.getNode().find('#btn-ch-0, #btn-ch-1, #btn-ch-2')).on('click', function() {
      self.curSearch = parseInt($(this).html()) - 1;
      self._updateSearchUI();
    });

    this.getItem('btn-ch-add').on('click', function() {
       if(self.data.searches.length < self.kMaxSearches) {
         self.data.searches.push({
           source: '',
           keywords: [
           ],
           filters: {
           }
         });
         self.curSearch = self.data.searches.length - 1;
         self._updateSearchUI();
       }
    });

    this.getItem('btn-ch-remove').on('click', function() {
       if(self.curSearch >= 0) {
         let index = self.curSearch;
         if(index === self.data.searches.length - 1) {
           --index;
         }
         self.data.searches.splice(self.curSearch, 1);
         self.curSearch = index;
         self._updateSearchUI();
       }
    });

    this.getItem('dropdown_source').on("shown.bs.dropdown", function() {
      $(this).find('#dropdown_source_list li a').on('click', function(sender) {
        if(self.curSearch >= 0) {
          let source = $(this).attr('id');
          self.data.searches[self.curSearch].source = source;
          let mediaSource = self._findMediaSource(source);
          let mediaSourceName = mediaSource == undefined ? '' : mediaSource.title;
          self.getItem('dropdown_source_btn').html(mediaSourceName + '&nbsp;<span class="caret"></span>');
        }
      });
    });

    this.getItem('keywords').change(function() {
      if(self.curSearch >= 0) {
        var tempKeywords = $(this).val().split('\n');
        var keywords = [];
        tempKeywords.forEach((item) => {
          if(item.match(self.regexKeyword) != null) {
            keywords.push(item);
          }
        });
        $(this).val(keywords.join('\n'));
        self.data.searches[self.curSearch].keywords = keywords;
      }
    });

    this.getItem('btn-create').on('click', function() {
      if(!self._validate()) {
        return;
      }
      if(self.eventHandler != null) {
        if(self.editMode === kChannelEditModeCreate) {
          self.eventHandler.onCreate(self.data);
        } else {
          self.eventHandler.onEdit(self.data);
        }
      }
      self.hide();
    });
  }

  _unadvice()
  {
    this.getItem('btn-create').off('');
    this.getItem('keywords').off();
    this.getItem('dropdown_source').off();
    this.getItem('dropdown_source').find('#dropdown_source_list li a').off();
    this.getItem('btn-ch-remove').off();
    this.getItem('btn-ch-add').off();
    $(this.getNode().find('#btn-ch-0, #btn-ch-1, #btn-ch-2')).off();
    this.getItem('channel_title').off();
  }

  _updateUI()
  {
    this.getItem('channel_title').val(this.data.title);
    this.getItem('dropdown_source_list').html(this._getMediaSourceListHtml());
  }

  _updateSearchUI()
  {
    for(let i = 0; i < this.kMaxSearches; i++) {
      this.getItem('btn-ch-' + i).css('display', i < this.data.searches.length ? 'block' : 'none');
    }
    this.getItem('btn-ch-add').css('display', this.data.searches.length < this.kMaxSearches ? 'block' : 'none');
    this.getItem('btn-ch-remove').css('display', this.data.searches.length > 0 ? 'block' : 'none');

    if(this.curSearch < 0) {
      this.getItem('dropdown_source_btn').html('<span class="caret"></span>');
      this.getItem('keywords').val('');
    } else {
      let search = this.data.searches[this.curSearch];

      let mediaSource = this._findMediaSource(search.source);
      let mediaSourceName = mediaSource == undefined ? '' : mediaSource.title;
      this.getItem('dropdown_source_btn').html(mediaSourceName + '&nbsp;<span class="caret"></span>');

      this.getItem('keywords').val(search.keywords.join('\n'));
    }
    this.getItem('dropdown_source_btn').prop('disabled', this.curSearch < 0);
    this.getItem('keywords').prop('disabled', this.curSearch < 0);
  }

  _validate()
  {
    if(this.data.title.length === 0) {
      this.getItem('channel_title').focus();
      return false;
    }
    if(this.data.searches.length === 0) {
      return false;
    }
    for(let index in this.data.searches) {
      let search = this.data.searches[index];
      if(search.source.length == 0 || search.keywords.length == 0) {
        return false;
      }
    }
    return true;
  }

  _getMediaSourceListHtml()
  {
    let html = '';
    if(this.mediaSources.length === 0) {
      html = '<li class="disabled"><a href="#">No Source</a></li>';
    } else {
      for(let i in this.mediaSources) {
        html+= '<li>' + '<a id="' + this.mediaSources[i].id + '" href="#">';
        html+= this.mediaSources[i].title + '</a></li>';
      }
    }
    return html;
  }

  _findMediaSource(source) {
    return this.mediaSources.find(function(elem) {
      return elem.id === source;
    });
  }

  _getMediaSources()
  {
    let sources = [];
    if(AppState.plugins != undefined) {
      for(let i in AppState.plugins) {
        let pluginData = AppState.plugins[i];
        if(pluginData.capabilities.indexOf('media-source') >= 0) {
          sources.push({
            id: pluginData.id,
            title: pluginData.title
          });
        }
      }
    }
    return sources;
  }
}

/** StaticView class. A view that displays static content based on template. */
class StaticView extends View
{
  constructor(config)
  {
    super(config.title);
    this.config = config;
  }

  getTemplate()
  {
    return this.config.template;
  }

  refresh()
  {
    this._render();
  }
  
  repaint()
  {
    this._render();
  }

  _render()
  {
    $('#main-content').html(this._buildHTML());
    this.onRender();
  }

  _buildHTML()
  {
    return $('#' + this.getTemplate())[0].innerHTML;
  }
}

/** DataView class. A view that can fetch and display dynamic data. */

/* Data view states */
const kStateIdle = 0;
const kStateLoading = 1;
const kStateEditing = 2;

/* Data view roles  */
const kRoleDisplay = 1<<0;
const kRoleEditor = 1<<1;

class DataView extends View
{
  constructor(config)
  {
    super(config.title);
    this.config = config;
    this.state = kStateIdle;
    this.data = null;
  }

  /** Returns the roles for this view. */
  getRoles()
  {
    return kRoleDisplay;
  }

  /** Returns the template id for row data.*/
  getRowTemplate()
  {
    return this.config.rowTemplate;
  }

  /** Returns the template id for end row data, if any.*/
  getEndRowTemplate()
  {
    return this.config.endRowTemplate;
  }

  /** Returns true if view is busy fetching data. */
  isBusy()
  {
    return this.state === kStateLoading;
  }

  /** Returns true if view is being edited. */
  isEditing()
  {
    return this.state == kStateEditing;
  }

  refresh()
  {
    if(!this.isBusy()) {
      this.state = kStateLoading;
      this.onFetchStart();
      $.get(this.config.apiEndPoint, this._onJqxhrFetchDone.bind(this))
       .always(this._onJqxhrFetchComplete.bind(this));
    }
  }

  repaint()
  {
    this._render();
  }
  
  beginEditMode()
  {
    if(this.state === kStateIdle && ((this.getRoles() & kRoleEditor) === kRoleEditor)) {
      this.state = kStateEditing;
      this.onEnterEditMode();
      return true;
    }
  }

  endEditMode()
  {
    if(this.state === kStateEditing) {
      this.onExitEditMode();
      this.state = kStateIdle;
    }
  }

  onFetchStart()
  {
    this.showLoader();
  }

  onFetchComplete()
  {
    this.hideLoader();
  }

  onEnterEditMode()
  { }

  onExitEditMode()
  { }

  _onJqxhrFetchDone(data, textStatus, jqxhr)
  {
    if(textStatus === 'success') {
      if(data.status === 'success') {
        let dsKey = this.config.dsKey;
        if(dsKey in data) {
          this.data = data[dsKey];
        } else {
          console.log('Datasource key "' + dsKey + '" not found.');
        }
      } else {
        console.log('Request failed with error: "' + data.message + '".');
      }
      console.log('Data = ' + JSON.stringify(data));
    } else {
      this.data = null;
      console.log('Request failed with status: ' + textStatus);
    }
  }

  _onJqxhrFetchComplete()
  {
    this.state = kStateIdle;
    this.onFetchComplete();
    this._render();
  }

  _render()
  {
    this.setContent(this._buildHTML());
    this.onRender();
  }

  _buildHTML()
  {
    let html =
    '<div class="view">' +
    '<div class="content vlayout">';
    if(this.data == null) {
      html+= '<p>No data available.</p>'
    } else {
      for(let index = 0; index < this.data.length; index++) {
        html+= this._buildRowHTML(this.data[index], index);
      }
      let endHtml = this._buildEndRowHTML();
      if(endHtml != null) {
        html+= endHtml;
      }
    }
    html+= '</div></div>';
    return html;
  }

  _buildRowHTML(data, index)
  {
    let html = $('#' + this.getRowTemplate())[0].innerHTML;
    return this._preprocessHtml(html, data, index);
  }

  _buildEndRowHTML()
  {
    let id = this.getEndRowTemplate();
    if(id != undefined) {
      return $('#' + id)[0].innerHTML;
    }
    return null;
  }

  _preprocessHtml(html, data, index)
  {
    html = html.replace(new RegExp('\\$index', "g"), index);
    let fields =
    ['id', 'title', 'author', 'enabled', 'version',
    'description', 'duration', 'thumbnailUrl'];
    for(let i in fields) {
      let MAX = 100;
      let fld = fields[i], value = data[fld];
      if (typeof(value) === 'string' && value.length > MAX) {
        value = value.substr(0, MAX) + '...';
      }
      html = html.replace(new RegExp('\\$data.' + fld, "g"), value);
    }
    return html;
  }
};

class MediaListView extends DataView
{
  constructor(channelId, channelTitle)
  {
    super({
      title: channelTitle,
      apiEndPoint: ApiFactory.listChannelUrl(channelId),
      dsKey: 'items',
      rowTemplate: 'template-channel-media-row-item'
      });
    this.channelId = channelId;
  }

  getEndRowTemplate()
  {
    if(this.data.length === 0) {
      return 'template-channel-media-row-noitems';
    }
  }

  onRender(html)
  {
    super.onRender();

    let self = this;

    let dropdown = $('.wrapper #content #main-content .dropdown');

    dropdown.on("show.bs.dropdown", function() {
      let mediaId = parseInt($(this).attr('id'));
      let list = self._getPlayerListHtml(self.data[mediaId].format);
      $(this).find('ul.dropdown-menu').html(list);
    });

    dropdown.on("shown.bs.dropdown", function() {
      let mediaId = parseInt($(this).attr('id'));
      $(this).find('ul li a').on('click',function(sender) {
        let player = $(this).attr('id');
        if(player !== undefined) {
          self._playMedia(mediaId, player, true);
        }
      });
    });

    $('.thumbnail').on('click', function() {
      let mediaId = parseInt($(this).attr('id'));
      let player = self._getDefaultPlayer(self.data[mediaId].format);
      if(player !== undefined) {
        self._playMedia(mediaId, player, false);
      }
    });
  }

  _playMedia(id, player, registerPlayer)
  {
    let mediaRequest = {
      title: this.data[id].title,
      mediaInfo: {
        "mediaUrl": this.data[id].mediaUrl,
        "format": this.data[id].format,
        "player": player,
        "registerPlayer": registerPlayer
      }
    };
    PlayerController.get().play(mediaRequest);
    this._showPlayer();
  }

  _showPlayer()
  {
    dlgPlayer = dlgPlayer || new PlayerDialog();
    dlgPlayer.show();
  }

  _getPlayerListHtml(format)
  {
    let playerList = '';
    let players = this._getPlayers(format);
    if(players.length === 0) {
      playerList = '<li class="disabled"><a href="#">No Player</a></li>';
    } else {
      let defaultIndex;
      for(let i in players) {
        if(players[i].id === this._getDefaultPlayer(format)) {
          defaultIndex = i;
        } else {
          playerList+= '<li>' + '<a id="' + players[i].id + '" href="#">';
          playerList+= players[i].title + '</a></li>';
        }
      }
      if(defaultIndex != undefined) {
        let defaultPlayer =
          '<li>' + '<a id="' + players[defaultIndex].id + '" href="#">' +
          '<b>' + players[defaultIndex].title + '</b></a></li>';
          playerList = defaultPlayer + playerList;
      }
    }
    return playerList;
  }

  _getDefaultPlayer(format)
  {
    if(AppState.mediaHandlers != undefined && format in AppState.mediaHandlers) {
      return AppState.mediaHandlers[format];
    }
  }

  _getPlayers(format)
  {
    let players = [];
    if(AppState.plugins != undefined) {
      for(let i in AppState.plugins) {
        let pluginData = AppState.plugins[i];
        if(pluginData.media_formats !== undefined &&
           pluginData.media_formats.indexOf(format) >= 0) {
          players.push({
            id: pluginData.id,
            title: pluginData.title
          });
        }
      }
    }
    return players;
  }
}

class ChannelsView extends DataView
{
  constructor()
  {
    super({
      title: 'Channels',
      apiEndPoint: ApiFactory.getChannelsUrl(),
      dsKey: 'channels',
      rowTemplate: 'template-channel-row-item',
      endRowTemplate: 'template-channel-row-item-new'
    });

    this.busy = false;
    this.dlgEdit = undefined;
    this.selectedRow = null;

    this.channelEditHandler =
    {
      onEdit: (data) => {
        console.log('Channel edit: ' + JSON.stringify(data, null, ' '));
        // let index = this.data.findIndex((item) => { return item.id === data.id });
        // this.data[index] = data;
        // #TODO: Call edit channel API.
        this._render();
      },

      onCreate: (channelData) =>
      {
        console.log('Channel create: ' + JSON.stringify(channelData, null, ' '));
        this.showLoader();
        this.state = kStateLoading;
        SimpleRequestBroker.post(ApiFactory.addChannelUrl(), JSON.stringify(channelData), (data) => {
          this.data === this.data || [];
          this.data.push(data.channel);
          this._render();
        }).always(() => {
          this.state = kStateIdle;
          this.hideLoader();
        });
      }
    };
  }

  getRoles()
  {
    return kRoleDisplay | kRoleEditor;
  }

  onRender(html)
  {
    super.onRender();

    let self = this;

    $('.row-item').find('#btn_edit').on('click', function() {
      if(self.state === kStateEditing) {
        let id = $(this).parent().parent().attr('id');
        self.state = kStateLoading;
        self.showLoader();
        SimpleRequestBroker.get(ApiFactory.getChannelUrl(id), (data) => {
          self._disposeEditDialog();
          self.dlgEdit = new ChannelDialog(kChannelEditModeEdit, data.channel);
          self.dlgEdit.setEventHandler(self.channelEditHandler).show();
        }).always(() => {
          self.hideLoader();
          self.state = kStateEditing;
        });
      }
    });

    $('.row-item').find('#btn_remove').on('click', function() {
      if(self.state === kStateEditing) {
        let id = $(this).parent().parent().attr('id');
        self.state = kStateLoading;
        self.showLoader();
        SimpleRequestBroker.get(ApiFactory.deleteChannelUrl(id), (data) => {
          let index = self.data.findIndex((item) => { return item.id === id });
          self.data.splice(index, 1);
          self._render();
        }).always(() => {
          self.hideLoader();
          self.state = kStateEditing;
        });
      }
    });

    $('.row-item').find('.text').on('mousedown', function() {
      if(self.state !== kStateLoading && self.isEditing()) {
        if($(this).parent().attr('id') !== '-1') {
          self._selectRow($(this).parent());
        }
      }
    });

    $('.row-item').find('.text').on('click', function() {
      if(self.state !== kStateLoading) {
        let id = $(this).parent().attr('id');
        if(id === '-1') {
          self._disposeEditDialog();
          self.dlgEdit = new ChannelDialog(kChannelEditModeCreate, undefined);
          self.dlgEdit.setEventHandler(self.channelEditHandler).show();
        } else if(!self.isEditing()) {
          let title = $(this).attr('title');
          ViewManager.get().pushView(new MediaListView(id, title));
        }
      }
    });
  }

  onEnterEditMode()
  {
    if($('.row-item').length > 0) {
      let row = $($('.row-item')[0]);
      if(row.attr('id') != '-1') {
        this._selectRow(row);
      }
    }
  }

  onExitEditMode()
  {
    this._clearSelection();
    this.selectedRow = null;
  }

  _disposeEditDialog()
  {
    if(this.dlgEdit != undefined) {
      this.dlgEdit.dispose();
      this.dlgEdit = undefined;
    }
  }

  _clearSelection()
  {
    if(this.selectedRow !== null) {
      this.selectedRow.removeClass('selected');
      $(this.selectedRow.find('.controls')[0]).css('display', 'none');
    }
  }

  _selectRow(row)
  {
    if(this.selectedRow == null || this.selectedRow.get(0) !== row.get(0)) {
      this._clearSelection();
      row.addClass('selected');
      $(row.find('.controls')[0]).css('display', 'block');
      this.selectedRow = row;
    }
  }
};

class PluginsView extends DataView
{
  constructor()
  {
    super({
      title: 'Plugins',
      apiEndPoint: ApiFactory.getPluginsUrl(),
      dsKey: 'plugins',
      rowTemplate: 'template-plugin-row-item'
      });
  }

  onFetchComplete()
  {
    super.onFetchComplete();
    if(this.data != null) {
      AppState.plugins = this.data;
    }
  }

  onRender(html)
  {
    super.onRender();

    let self = this;

    $('.row-item').on('click', function(sender) {
      let id = $(this).attr('id');
      console.log(id);
      let plugins = self.data;
      for(let i in plugins) {
        let info = plugins[i];
        if(info.id === id) {
          console.log(info.description);
          break;
        }
      }
    });
  }
};

class MainView extends StaticView
{
  constructor()
  {
    super({title: 'Mobicast', template: 'template-main'});
  }
}

class AboutView extends StaticView
{
  constructor()
  {
    super({'title': 'About', template: 'template-about'});
  }
}

class SideBar
{
  constructor()
  {
    $('#sidebar_button').on('click', () => {
      this.show();
    });    
    $('#sidebar_dismiss_button, .overlay').on('click', () => {
      this.hide();
    });
    $('#nav_left_button').on('click', () => {
      ViewManager.get().popView();
    });
    this._initMenu();
  }

  show()
  {
    $('#sidebar').addClass('active');
    $('.overlay').fadeIn();
    $('.collapse.in').toggleClass('in');
    $('a[aria-expanded=true]').attr('aria-expanded', 'false');
  }

  hide()
  {
    $('#sidebar').removeClass('active');
    $('.overlay').fadeOut();
  }

  _initMenu()
  {
    let handler = this._onClickMenu.bind(this);
    $('#menu_channels').on('click', handler);
    $('#menu_about').on('click', handler);
    $('#menu_plugins').on('click', handler);
  }

  _onClickMenu(sender)
  {
    let view;
    if(sender.target.id === 'menu_channels') {
      view = new ChannelsView();
    } else if(sender.target.id === 'menu_about') {
      view = new AboutView();
    } else if(sender.target.id === 'menu_plugins') {
      view = new PluginsView();
    }
    if(view != undefined) {
      ViewManager.get().setView(view);
      this.hide();
    }
  }
}

class ViewNode
{
  constructor(view)
  {
    this.view = view;
    this.next = this.previous = null;
  }
}

class ViewManager
{
  constructor()
  {
    this.mainContent = $('#main-content');
    this.last = null;
    this.setView(new MainView());

    $('#edit_button').on('click', function() {
      let view = ViewManager.get().getCurrentView();
      if(view != null && !view.isBusy()) {
        if(view.isEditing()) {
          view.endEditMode();
        } else {
          view.beginEditMode();
        }
        if(view.isEditing()) {
          $(this).addClass('active');
        } else {
          $(this).removeClass('active');
        }
        $(this).blur();
      }
    });
  }

  static get()
  {
    ViewManager.instance = ViewManager.instance || new ViewManager();
    return ViewManager.instance;
  }

  static destroy()
  {
    ViewManager.instance = undefined;
  }

  getCurrentView()
  {
    return this.last === null ? null : this.last.view;
  }

  setView(view)
  {
    this._endEditingMode();
    this._removeAllViews();
    this.last = new ViewNode(view);
    this._showView(view, true);
  }

  pushView(view)
  {
    this._endEditingMode();
    let node = new ViewNode(view);
    if(this.last !== null) {
      this.last.next = node;
      node.previous = this.last;
      this._hideView(this.last.view);
    }
    this.last = node;
    this._showView(view, true);
  }

  popView()
  {
    this._endEditingMode();    
    this._disposeView(this.last.view);
    let previous = this.last.previous;
    if(previous != null) {
      previous.next = null;
    }
    this.last = previous;
    this._showView(this.last.view);
  }

  _showView(view, refresh = false)
  {
    $('#main-title').html(view.getTitle());
    this.mainContent.html('');
    
    this._adjustContentOffset();
    this._updateNavBarUI();
    
    view.onShow();
    if(refresh === true) {
      view.refresh();
    } else {
      view.repaint();
    }
  }

  _hideView(view)
  {
    view.onClose();
    this.mainContent.html('');
  }

  _disposeView(view)
  {
    this._hideView(view);
    view.onDestroy();
  }

  _removeAllViews()
  {
    while(this.last !== null) {
      this._disposeView(this.last.view);
      this.last = this.last.previous;
    }
  }

  _adjustContentOffset()
  {
    let barHeight = $('.navbar-top').outerHeight();
    this.mainContent.css('top', barHeight + 'px')
  }

  _updateNavBarUI()
  {
    let current = this.last;    
    if(current.previous == null) {
      $('#sidebar_button').css('display', 'block');
      $('#nav_left_button').css('display', 'none');
    } else {
      $('#sidebar_button').css('display', 'none');
      $('#nav_left_button').css('display', 'block');
    }
    let view = current.view;
    if(view instanceof DataView && ((view.getRoles() & kRoleEditor) === kRoleEditor)) {
      $('#edit_button').show();
    } else {
      $('#edit_button').hide();
    }
  }

  _endEditingMode()
  {
    let view = this.getCurrentView();
    if(view !== null && view instanceof DataView && view.isEditing()) {
      view.endEditMode();
    }
    $('#edit_button').removeClass('active');
    $('#edit_button').blur();
  }
}

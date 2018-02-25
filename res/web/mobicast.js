/*
 * mobicast.js
 *
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
var MC = null;

window.onload = function() {
  try {
    var _MODULE_ = 'MobiCast';

    if(typeof _mobicast_engine == 'undefined') {
      document.getElementById('content').innerHTML =
      '<p>This page requires <a href="https://github.com/bal-chettri/mobicast">MobiCast</a> software.</p>';
      return false;
    }

    MC = {};
    
    var isIE = navigator.userAgent.match("MSIE ([0-9]{1,}[\.0-9]{0,})");
    MC.isIE = isIE && isIE.length > 0;

    MC.$ = function(id) {
      return document.getElementById(id);
    }
    MC.unescape = function(str) {
      str = str.replace('\n', '<br />');
      return str;
    }
    MC.toJsArray = function(arr) {
      return MC.isIE ? (new VBArray(arr)).toArray() : arr;
    }
    // #Deprecated, use toNativeArray instead.
    MC.toVbArray = function(arr) {
      if(MC.isIE) {
        var dict = new ActiveXObject("Scripting.Dictionary");
        var index = 0;
        for(var i in arr) {
          dict.add(index++, arr[i]);
        }
        return dict.Items();
      }
      return arr;
    }
    MC.toNativeArray = function(arr) {
      if(MC.isIE) {
        var dict = new ActiveXObject("Scripting.Dictionary");
        var index = 0;
        for(var i in arr) {
          dict.add(index++, arr[i]);
        }
        return dict.Items();
      }
      return arr;
    }

    // Show boot logo.
    MC.$('content').innerHTML = 
    '<table width="100%" height="100%"><tr><td align="center"><img src="logo.png" /></td></tr></table>';

    // Mobicast core objects.
    MC.engine = _mobicast_engine;    
    MC.storage = _mobicast_storage;
    MC.db = _mobicast_db;
    MC.mm = _mobicast_mm;

    // MobiCast flags and constants.
    MC.VERSION = MC.engine.version;
    MC.DEBUG = MC.engine.debug;

    MC.kMediaTypeInvalid =  0;
    MC.kMediaTypeImage = 1;
    MC.kMediaTypeMusic = 2;
    MC.kMediaTypeVideo = 4;
    MC.kMediaTypeAll = MC.kMediaTypeImage | MC.kMediaTypeMusic | MC.kMediaTypeVideo;

    // Plugin features
    MC.kPluginCapabilityMediaSource = 1;
    MC.kPluginCapabilityPlayer = 2;
    
    // Plugin media search filters
    MC.kPluginMediaSearchFilterType = 'type';
    MC.kPluginMediaSearchFilterDateRange = 'date-range';
    MC.kPluginMediaSearchFilterSubtitles = 'subtitles';
    MC.kPluginMediaSearchFilterQuality = 'quality';
    MC.kPluginMediaSearchFilterMax = 'max';
    MC.kPluginMediaSearchFilterGenre = 'genre';
    MC.kPluginMediaSearchFilterYear = 'year';
    
    // Player states
    MC.kPlayerStateReady = 0;
    MC.kPlayerStateLoading = 1;
    MC.kPlayerStatePlaying = 2;
    MC.kPlayerStatePaused = 3;
    MC.kPlayerStateFinished = 4;

    MC.kDbKeySetupFlag = 'mobicast.setup.flag';
    MC.kDbKeyDriveVol = 'mobicast.drive.vol';
    MC.kDbKeyPhoneId = 'mobicast.phone.id';
    MC.kDbKeyPhoneName = 'mobicast.phone.name';
    MC.kDbKeyPhoneStatus = 'mobicast.phone.status';
    MC.kDbKeyPhonePasscode = 'mobicast.phone.passcode';

    MC.kNotificationFadeInTime = 1000;
    MC.kNotificationFadeOutTime = 3000;
    MC.kNotificationTimeout = 3000;

    // Debugging APIs
    if(MC.DEBUG) {
      MC.$('debug').style.display = 'block';
    }
    MC.print = function(text) {
      if(MC.DEBUG) {
        var console = MC.$('console');
        console.innerHTML = console.innerHTML + MC.unescape(text);
      }
    }
    MC.log_ = function(mod, type, text) {
      if(MC.DEBUG) {
        var html = '<span color="#ffe0e0">' + mod + '</span>/' + type + ':&nbsp';
        html+= text;
        MC.print(html + '<br />');
        // Log to native debugger.
        MC.engine.log(mod + '/' + type + ': ' + text);
      }
    }
    MC.logd = function(mod, text) {
      MC.log_(mod, 'D', text)
    }
    MC.logi = function(mod, text) {
      MC.log_(mod, 'I', text)
    }
    MC.loge = function(mod, text) {
      MC.log_(mod, 'E', text)
    }
    MC.loga = function(mod, text) {
      MC.log_(mod, 'A', text)
    }
    // MC.log defaults to MC.logi.
    MC.log = MC.logi;

    // Dump Ids of elements matching tag.
    MC.dump = function(tag) {
      MC.log(_MODULE_, 'Dumping "' + tag + '"');
      var items = document.getElementsByTagName(tag);
      for(var index in items) {
          MC.logd(_MODULE_, '#DUMP: ' + items[index].getAttribute('id') + '<br /><br />');
      }
    }

    // Script APIs
    MC._curScript = null;
    MC._writeBuff = null;
    // Writes to the parent element replacing it.
    MC.write = function(html) {
      if(MC._writeBuff == null) {
        MC._writeBuff = html;
      } else {
        MC._writeBuff = MC._writeBuff + html;
      }
    }
    MC.runScripts = function(root) {
      if(!root) {
        root = MC.$('content');
      }
      var domScripts = root.getElementsByTagName('script');
      if(domScripts.length > 0) {
        // Replacing <script> directly removes it from DOM immediately
        // so use a temporary array.
        var scripts = [];
        for(var index = 0; index < domScripts.length; index++) {
          scripts.push(domScripts[index]);
        }
        for(var index in scripts) {
          MC._curScript = scripts[index];
          MC._writeBuff = null;

          var parentElem = MC._curScript.parentElement;
          var script = document.createElement('script');
          script.type = 'text/javascript';
          script.text = MC._curScript.innerHTML;
          parentElem.replaceChild(script, MC._curScript);

          if(MC._writeBuff != null) {
            script.outerHTML = MC._writeBuff;
          }
        }
        MC._curScript = MC._writeBuff = null;
      }
    }

    // Send HTTP request with advanced options.
    // Supported fields are:
    // [Options] method = GET | POST etc. Default is GET.
    // [Optional] async = true | false
    // [Optional] headers = object
    // [Optional] body = request body for POST
    MC.httpReq = function(url, options, callback) {
      var method = 'method' in options ? options['method'] : 'GET';
      var async = 'async' in options ? options['async'] : true;
      var headers = 'headers' in options ? options['headers'] : null;
      var body = 'body' in options ? options['body'] : null;
      var xhttp = new XMLHttpRequest();
      if(xhttp == null) {
        return false;
      }
      xhttp.onreadystatechange = function() {
        if(this.readyState == 4) {
          MC.log(_MODULE_, 'URL "' +  url + '" loaded. Status code = ' + xhttp.status);
          callback(this);
        }
      };
      xhttp.open(method, url, async);
      if(headers != null) {
        for(name in headers) {
          xhttp.setRequestHeader(name, headers[name]);
        }
      }
      xhttp.send(body);
      return true;
    }
    // Send simple HTTP GET request.
    MC.httpGet = function(url, callback) {
      var options = { };
      return MC.httpReq(url, options, callback);
    }

    // Dynamic script loading
    MC.included = function(src) {
      var head = document.getElementsByTagName('head')[0];
      for(var i = 0; i < head.childNodes.length; i++) {
        var node = head.childNodes[i];
        if(node.tagName == 'SCRIPT' && (node.src == src || node._src == src)) {
          return true;
        }
      }
      return false;
    }
    MC.include = function(path, options) {
      if(MC.included(path)) {
        MC.log(_MODULE_, ' Script "' + path + '" already loaded.');
        return;
      }
      var script = document.createElement('script');
      script.type = 'text/javascript';
      /*script.onload = function() {
        alert('script.onload: script loaded');
      }
      script.onreadystatechange = function() {
        MC.log(_MODULE_, 'script.onreadystatechange: state = ' + script.readyState);
      }*/
      if(options == undefined) {
        script.src = path;
        document.getElementsByTagName('head')[0].appendChild(script);
      } else {
        MC.httpReq(path, options, function(xhttp) {
          if(xhttp.status == 200) {
            MC.log(_MODULE_, 'Script "' +  path + '" loaded');
            script.text = xhttp.responseText;
            script._src = path;
            document.getElementsByTagName('head')[0].appendChild(script);
          }
        });
      }
    };

    // Service APIs
    MC._serviceMap = {};
    MC._serviceToken = MC.engine.serviceToken;
    MC.loadService = function(path) {
      MC.log(_MODULE_, 'Loading service "' + path + '"');
      // Service scripts must be requested with a valid access-token for the
      // web server to send them instead of executing.
      var httpOptions = {
        'headers': {
          'access-token': MC._serviceToken
        }
      };
      MC.include(path, httpOptions);
    }
    MC.loadServices = function() {
      var services = MC.toJsArray(MC.engine.getServices());
      MC.log(_MODULE_, 'Loading services...(' + services.length + ') found');
      for(var i in services) {
        MC.loadService(services[i]);
      }
    }
    MC.registerService = function(path, srvc) {
      MC.log(_MODULE_, 'Registering service "' + path + '", typeof srvc = ' + typeof srvc);
      if(typeof srvc == 'function') {
        MC._serviceMap[path] = srvc;
      } else {
        MC.loge('Invalid service ' + path);
      }
    }
    MC.runService = function(path, _req, _resp) {
      if(path in MC._serviceMap) {
        return MC._serviceMap[path](_req, _resp);
      } else {
        MC.loge(_MODULE_, 'Service "' + path + '" not registered.');
        return false;
      }
    }

    // Plugin APIs
    MC._plugins = [];
    MC._curPlugin = null;
    MC.getActivePlugin = function() {
      return MC._curPlugin;
    }
    MC.findPlugin = function(id) {
      for(var i in MC._plugins) {
        var pinfo = MC._plugins[i];
        if(pinfo.meta.id == id) {
          return pinfo;
        }
      }
      return null;
    }
    MC.getPlugins = function() {
      return MC._plugins;
    }
    MC.registerPlugin = function(id, p) {
      MC.log(_MODULE_, 'Registering plugin "' + id + '"');
      var pinfo = MC.findPlugin(id);
      if(pinfo == null) {
        MC.loge(_MODULE_, 'Plugin "' + id + '" not found in plugin list.');
      } else if(pinfo.plugin != undefined) {
        MC.loge(_MODULE_, 'Plugin "' + id + '" already registered.');
      } else {
        pinfo.plugin = p;
        return true;
      }
      return false;
    }
    MC.activatePlugin = function(id) {
      MC.loge(_MODULE_, 'Activating plugin "' + id + '"');
      var pinfo = MC.findPlugin(id);
      if(pinfo == null) {
        MC.loge(_MODULE_, 'Plugin "' + id + '" not found in plugin list.');
        return false;
      } else if(pinfo.plugin == undefined) {
        MC.loge(_MODULE_, 'Plugin "' + id + '" not registered.');
        return false;
      }
      if(pinfo != MC._curPlugin) {
        if(MC._curPlugin != null) {
          MC._curPlugin.plugin.onClose();
          MC.clearContent();
        }
        MC.loadContent(pinfo.meta.player);
        MC._curPlugin = pinfo;
        pinfo.plugin.onLoad();
      }
      return true;
    }
    MC.loadPlugin = function(pm) {
      MC.log(_MODULE_, 'Loading plugin "' + pm.id);
      MC.include(pm.main);
    }
    MC.loadPlugins = function() {
      MC.log(_MODULE_, 'Loading plugins...');
      var pluginCount = MC.engine.pluginCount;
      for(var i = 0; i < pluginCount; i++) {
        var pm = MC.engine.getPluginMeta(i);
        MC._plugins.push({'meta': pm});
      }
      for(var i in MC._plugins) {
        var pm = MC._plugins[i].meta;
        var capabilities = pm.capabilities;
        var strCaps = "[";
        if(capabilities & MC.kPluginCapabilityMediaSource) {
          strCaps+= "media-source, "
        }
        if(capabilities & MC.kPluginCapabilityPlayer) {
          strCaps+= "player"
        }
        strCaps+= "]";
        MC.log(_MODULE_, 'Plugin "' + pm.id + 
          '", ' + pm.title + 
          ', desc=' + pm.description + 
          ', enabled=' + pm.enabled + 
          ', caps=' + strCaps);          
        if(pm.enabled) {
          MC.loadPlugin(pm);
        }
      }
    }

    // Content APIs
    MC.setContent = function(html) {
      var prevContent = MC.$('content');
      var parentElem = prevContent.parentElement;

      var content = document.createElement('div');
      content.id = 'content';
      content.innerHTML = html;
      
      parentElem.replaceChild(content, prevContent);
    }
    MC.loadContent = function(url) {
      MC.httpGet(url, function(xhttp) {
        MC.setContent(xhttp.responseText);
        MC.runScripts();
      });
    }
    MC.clearContent = function() {
      MC.$('content').innerHTML = '';
    }

    // UI APIs
    MC.ui = { };
    MC.ui.toggle = function(id) {
      var elem = MC.$(id);
      elem.style.display = elem.style.display == 'block' ? 'none' : 'block';
    }
    MC.ui.list = { };
    MC.ui.list.setSelected = function(li) {
      var item = li.parentElement;
      item = item.firstChild;
      while(item != null) {
        if(item != li) {
          item.className = 'normal';
        }
        item = item.nextSibling;
      }
      li.className = 'selected';
    }
    MC.ui.list.getSelected = function(id) {
      var list = MC.$(id);
      var item = list.firstChild;
      while(item != null) {
        if(item.className == 'selected') {
          return item;
        }
        item = item.nextSibling;
      }
      return null;
    }

    // Notification APIs
    MC.notify = function(msg) {
      MC.log(_MODULE_, 'Showing notification bar. Message = ' + msg);
      MC.$('notiftext').innerHTML = msg;
      $('#notifbar').fadeIn(MC.kNotificationFadeInTime);
      setTimeout(function() {
        MC.log(_MODULE_, 'Hiding notification bar.');
        $('#notifbar').fadeOut(MC.kNotificationFadeOutTime);
      }, MC.kNotificationTimeout);
    }

    // Storage APIs
    MC.mountStorage = function(driveVol) {
      var volIndex = -1;
      if(driveVol != null) {
        MC.log(_MODULE_, 'Mounting storage "' + driveVol + '"');
        var volumes = MC.toJsArray(MC.storage.getVolumes());
        MC.log(_MODULE_, 'Available volumes(' + volumes.length + ')');
        for(var i = 0; i < volumes.length; i++) {
          MC.log(_MODULE_, volumes[i] + '');
          if(volumes[i].split(',')[0] == driveVol) {
            volIndex = i;
          }
        }
        if(volIndex == -1) {
          MC.log(_MODULE_, 'Volume "' + driveVol + '" not found.');
        } else {
          var volInfo = volumes[volIndex].split(',');
          if(MC.storage.mount(volInfo[1])) {
            MC.log(_MODULE_, 'Successfully mounted volume "' + driveVol + '"');
          } else {
            MC.log(_MODULE_, 'Failed to mount volume "' + driveVol + '".');
          }
        }
      }
      return MC.storage.mounted;
    }

    ////////////////////////////////////////////////////////////////////////////
    
    MC.log(_MODULE_, 'Mobicast version ' + MC.VERSION);
    MC.log(_MODULE_, 'DEBUG = ' + (MC.DEBUG ? 'True' : 'False') + '\n\n');
    MC.log(_MODULE_,
      'System info:\n\n' +
      'Cookies: ' + navigator.cookieEnabled + '<br />' +
      'AppName:  ' + navigator.appName + '<br />' +
      'AppCodeName: ' + navigator.appCodeName + '<br />' +
      'Product: ' + navigator.product + '<br />' +
      'AppVersion: ' + navigator.appVersion + '<br />' +
      'UserAgent: ' + navigator.userAgent + '<br /><br />'  +
      'MC.isIE: ' + MC.isIE + '<br />' + 
      'PostMessage support: ' + (typeof window.postMessage == 'function' ? 'Present' : 'Missing') + '<br />' +
      'Typed array support: ' + (typeof Uint8Array == 'function' ? 'Present' : 'Missing') + '<br />' + 
      'btoa support: ' + (typeof window.btoa == 'function' ? 'Present' : 'Missing') + '<br />' +
      '<br />'
    );
    
    MC.log(_MODULE_, 'Initializing database...');
    MC.db.open();

    MC.log(_MODULE_, 'Checking drive...');
    var driveVol = MC.db.getProp(MC.kDbKeyDriveVol);
    if(driveVol == null) {
      MC.log(_MODULE_, 'Drive not configured.');
    } else {
      MC.log(_MODULE_, 'Drive volume = ' + driveVol);
      MC.mountStorage(driveVol);
    }

    // Load all registered plugins.
    MC.loadPlugins();

    // Load all registered services.
    MC.loadServices();

    // Finally, call mc_main() to start the app.
    if(typeof mc_main != 'function') {
      MC.loge(_MODULE_, 'mc_main is undefined.');
    } else {
      MC.log(_MODULE_, 'Calling mc_main.');
      mc_main();
    }
  }
  catch(e) {
    var msg = 'Exception caught: ' + e.message + ', Line no: ' + e.lineNumber;
    alert(msg);
    MC.engine.log(msg);
  }
};

window.onerror = function(msg, url, lineNo, columnNo, error) {
  var string = msg.toLowerCase();
  var substring = "script error";
  var errmsg;
  if (string.indexOf(substring) > -1){
      errmsg = 'Script Error occured.';
  } else {
      var message = [
          'Message: ' + msg,
          'URL: ' + url,
          'Line: ' + lineNo,
          'Column: ' + columnNo
      ].join(', ');      
      errmsg = 'Script Error occured. ' + message;
  }
  alert(msg);
  MC.engine.log(msg);
  return false;
}

// Native/js bridge.
function _mc_js_run_service(path, _req, _resp) {
  return MC.runService(path, _req, _resp);
}

function _mc_js_get_plugin_media_source(pluginid) {
  var pm = MC.findPlugin(pluginid);
  if(pm != null && (pm.meta.capabilities & MC.kPluginCapabilityMediaSource) &&
     pm.plugin != undefined) {
    return pm.plugin.getMediaSource();
  }
}

function _mc_js_plugin_get_search_url(pluginid, keywords, filter) {
  var mediaSource = _mc_js_get_plugin_media_source(pluginid);
  if(mediaSource != null) {
    return mediaSource.getSearchURL(keywords, filter);
  }
}

function _mc_js_plugin_get_media_extraction_tags(pluginid) {
  var mediaSource = _mc_js_get_plugin_media_source(pluginid);
  if(mediaSource != null) {
    var tags = mediaSource.getMediaExtractionTags();
      tags = MC.toNativeArray([
        MC.toNativeArray(tags.videoURL),
        MC.toNativeArray(tags.thumbnailURL),
        MC.toNativeArray(tags.duration),
        MC.toNativeArray(tags.title)
      ]);
      return tags;
  }  
}

function _mc_js_plugin_on_media_item_found(pluginid, media) {
  var mediaSource = _mc_js_get_plugin_media_source(pluginid);
  if(mediaSource != null) {
    mediaSource.onMediaItemFound(media);
  }  
}

// JSON
if(typeof(JSON) == 'undefined') {
  JSON = {}

  JSON._quote = function(s) {
    return '"' + s + '"';
  }

  JSON._encode = function(v) {
    var t = typeof v;
    var valJson;
    if(t == 'boolean' || t == 'number') {
      valJson = v.toString();
    } else if(t == 'string') {
      valJson = JSON._quote(v.toString());
    } else if(t == 'object') {
      valJson = '';
      if(v.constructor == Array) {
        valJson = JSON._encodeArray(v);
      } else {
        valJson = JSON._encodeObject(v);
      }
    }
    return valJson;
  }

  JSON._encodeArray = function(arr) {
    var json = '[';
    var count = 0;
    for(var i = 0 ; i < arr.length; i++) {
      var valJson = JSON._encode(arr[i]);
      if(typeof valJson != 'undefined') {
        if(count > 0) {
          json+= ',';
        }
        json+= valJson;
        ++count;
      }
    }
    json+= ']';
    return json;
  }      

  JSON._encodeObject = function(obj) {
    var json = '{';
    var fields = 0;
    for(var key in obj) {
      if(fields > 0) {
        json+= ',';
      }
      var valJson = JSON._encode(obj[key]);
      if(typeof valJson == 'undefined') {
        continue;
      }
      json+= JSON._quote(key);
      json+= ': ';
      json+= valJson;
      ++fields;
    }
    json+= '}';
    return json;
  }

  JSON.stringify = function(obj) {
    if(typeof obj == 'object' && obj.constructor != Array) {
      return JSON._encodeObject(obj);
    }
  }
  
  JSON.parse = function(json) {
    if(typeof json == 'string') {      
      eval('var __jsonobj=' + json + ';');
      return __jsonobj;
    }
  }
}

// BASE64 works with Array of bytes(Array) and String.
if(typeof(BASE64) == 'undefined') {
  BASE64 = { }
  
  // Returns base64 encoded string from array of bytes.
  BASE64.to = function(arr) {
    var str = String.fromCharCode.apply(null, arr);
    return Base64.encode(str);
  }
  
  // Returns array of bytes from base64 encoded string.
  BASE64.from = function(str) {
    // return Array.from(Base64.decode(str));
    return Base64.decode(str).split("");
  }
}

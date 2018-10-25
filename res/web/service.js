/*
 * service.js
 *
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
var HttpUtils = {
  parseQueryString: function(rqs) {
    var items = rqs.split('&');
    var qs = {
      items: { },
      get: function(key) {
        return (key in this.items) ? this.items[key] : null;
      }
    };
    for(i in items) {
      var kv = items[i].split('=');
      qs.items[kv[0]] = kv[1];
    }
    return qs;
  }
};

var OBJECT = {
  isObject: function(v) {    
    return v != undefined && typeof v == 'object' && v.constructor != Array;
  },
  
  isArray: function(v) {
    return v != undefined && typeof v == 'object' && v.constructor == Array;
  },
  
  has: function(obj, key, type) {
    if(!(key in obj)) {
      return false;
    }
    if(type == 'array') {
     return this.isArray(obj[key]);
    }
    else if(type == 'object') {
     return this.isObject(obj[key]);
    }
    else {
      return typeof obj[key] == type;
    }
  }
};

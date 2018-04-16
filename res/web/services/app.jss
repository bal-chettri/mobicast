/*
 * app.jss
 *
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */

//
// PairPhoneAPI.
//
var PairPhoneAPI = {
  main: function(_req, qs) {
    var div = MC.$('_mc_phone_setup');
    if(div == null || div == undefined) {
      _req.replyText(200, this._errorResult('Setup is not running.'), "application/json");
    } else {
      var phone_id = qsGet(qs, 'id');
      var phone_name = qsGet(qs, 'name');
      if(phone_id == null || phone_id.length == 0 ||
         phone_name == null || phone_name.length == 0)
      {
        div.innerHTML = 'Waiting for your phone to connect...';
        _req.replyText(200, this._errorResult('Invalid phone data.'), "application/json");
      }
      else
      {
        MC.db.setProp(MC.kDbKeyPhoneId, phone_id);
        MC.db.setProp(MC.kDbKeyPhoneName, phone_name);
        MC.db.setProp(MC.kDbKeyPhoneStatus, 1);
        var passcode = this._genPasscode();
        MC.db.setProp(MC.kDbKeyPhonePasscode, passcode);
        
        div.innerHTML = 
        'Phone:&nbsp;<b>' + phone_name + '</b><br />' +
        'Enter passcode <b>' + passcode + '</b> and tap Activate on your phone.';
        
        var result = {
          'status': 'success'
        }
        _req.replyText(200, JSON.stringify(result), "application/json");
      }
    }    
    return true;
  },
  
  _genPasscode: function() {   
    var kMaxDigits = 4;
    var code = '';
    for(var i = 0; i < kMaxDigits; i++) {
      var d = Math.floor(Math.random() * 10);
      code+= d.toString();
    }    
    return code;
  },
  
  _errorResult: function(msg) {
    return JSON.stringify({
      'status': 'error',
      'message': msg
    });
  }
};

var ActivatePhoneAPI = {
  main: function(_req, qs) {
    var div = MC.$('_mc_phone_setup');
    if(div == null || div == undefined) {
      _req.replyText(200, this._errorResult('Setup is not running.'), "application/json");
    } else {
      var phoneId = qsGet(qs, 'id');
      var passcode = qsGet(qs, 'code');
      if(phoneId == null || phoneId.length == 0 || 
         passcode == null || passcode.length != 4)
      {
        _req.replyText(200, this._errorResult('Invalid activation data.'), "application/json");
      }
      else
      {
        if(phoneId != MC.db.getProp(MC.kDbKeyPhoneId) || 
           passcode != MC.db.getProp(MC.kDbKeyPhonePasscode) || 
           MC.db.getProp(MC.kDbKeyPhoneStatus) != 1)
        {
          _req.replyText(200, this._errorResult('Invalid pairing data.'), "application/json");
        } else {
          MC.db.setProp(MC.kDbKeyPhoneStatus, 2);
          div.innerHTML = 'Phone <b>' + MC.db.getProp(MC.kDbKeyPhoneName) + '</b> is paired now.'
        }
      }
      
      var result = {
        'status': 'success'
      }
      _req.replyText(200, JSON.stringify(result), "application/json");
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
MC.registerService("/services/app.jss", function(_req, _resp) {
  try {
    var qs = parseQueryString(_req.rawQueryString);
    var cmd = qsGet(qs, 'cmd');

    if(cmd == 'pair_phone')
    {
      return PairPhoneAPI.main(_req, qs);
    }
    else if(cmd == 'activate_phone')
    {
      return ActivatePhoneAPI.main(_req, qs);
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

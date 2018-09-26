/*
 * boot.js
 *
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
function mc_main() {
  var _MODULE_ = 'MobiCast';
  
  MC.log(_MODULE_, 'Initializing...');
  
  var setup = MC.db.getProp(MC.kDbKeySetupFlag) != 1;
  setup = setup || !MC.storage.mounted;
  
  if(setup) {
    MC.log(_MODULE_, 'Running setup...');
    MC.setup = { };
    MC.setup.init = function() {  
      this.views = ['setup-drive.html', 'setup-phone.html'];
      this.index = 0;
      return this;
    }
    MC.setup.next = function() {
      if(this.index == this.views.length) {
        MC.db.setProp(MC.kDbKeySetupFlag, 1);
        MC.loadContent('home.html');
      } else {
        var view = 'setup/' + this.views[this.index++];
        MC.log(_MODULE_, 'Loading setup ' + this.index + ' of ' + 
               this.views.length + ': ' + view);
        MC.loadContent(view);
      }
    }
    MC.setup.init().next();
  } else {
    MC.loadContent('home.html');
  }
}

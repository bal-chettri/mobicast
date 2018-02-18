/*
 * setup.js
 *
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
function onSaveDriveSettings() { 
  var item = MC.ui.list.getSelected('setup-drive-list');
  if(item == null) {
    MC.notify('Select a drive to use with Mobicast.');
  } else {
    var vol = item.getAttribute('vol');
    MC.db.setProp(MC.kDbKeyDriveVol, vol);
    if(MC.mountStorage(vol)) {
      MC.notify('Storage "' + vol + '" is ready for use.');
      MC.setup.next();
    } else {
      MC.notify('Failed to mount drive with volume label ' + vol);
    }
  }
}

function onSavePhoneSettings() {
  MC.notify('Your phone is ready for use with MobiCast.');
  MC.setup.next();
}

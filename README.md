# README

## MobiCast
MobiCast is a smart-phone controlled media viewing experience on you PC/Mac. Using plugin based architecture, it allows you to catalog media content from various sources from the Internet or even local drive, organize them into Channels and watch them at you fingertip.

MobiCast is supported for Windows and MacOS and is implemented in C++/ObjC/JavaScript.

## Branches and Releases
You'll need to manually build the code to try it out. Binary releases are not supported.

'dev' is the active development branch where the most updated, but possibly unstable, code resides. You can checkout the development branch to build with the latest features and updates.

'master' is the latest stable branch. Checkout the 'master' branch to build from the most stable code.

## Dependencies
To build MobiCast, you'll need following tools:

* 'cygwin' to remotely build for MacOS from a Windows system.

* python 2.7

* 'mod-pbxproj' python tool available at https://github.com/kronenthaler/mod-pbxproj. Follow the wiki under Installation section to install it on your local system.

## Building MobiCast
Currently, building MobiCast is supported only for MacOS. You can also build from a Windows system remotely, however you need a MacOS system to build the code. Follow the steps below to build:

* Checkout or update your code from the 'dev' or 'master' branch.  

* Go to builds/mobicast directory:
```
  cd builds/mobicast
```
  
* Run xcode-build-remote.sh script to build the code remotely.
```
  chmod +x xcode-build-remote.sh
  ./xcode-build-remote.sh <user-name> <host-name>
```
  
  where 'user-name' is the login name and 'host-name' is the IP address of your remote MacOS system.
  
  The MacOS build tree is generated at /Users/user-name/mobicast-build
  
## License
This project is licensed under GNU General Public License.

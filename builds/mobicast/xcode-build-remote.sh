#!/bin/sh
#
# Copyright (C) 2018  Bal Chettri
# Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
#
# Usage: xcode-build-remote.sh username hostname
#
if [ $# -ne 2 ]; then
    echo Usage: $0 username hostname
    exit
fi

REMOTE_USER=$1
REMOTE_HOST=$2

# Build project.
python xcode-gen.py
if [ $? -ne 0 ]; then
    exit
fi

LOCAL_PATH=../../
REMOTE_PATH=/Users/$REMOTE_USER/mobicast-build

# Exclude these folders/files.
EXCLUDES=\
".git "\
".gitignore "\
".gitattributes "\
"*.cmd "\
"bin "\
"docs "\
"builds/mobicast/xcode-gen.py "\
"builds/mobicast/xcode-build-remote.sh "\
"builds/mobicast/proj_vs2010 "\
"builds/mobicast/proj_xcode_template "\
"builds/mobicast-client "\
"lib/win32 "\
"lib/win64"

# Generated; Exclude from deletion(required for incremental builds)
EXCLUDES="$EXCLUDES builds/mobicast/proj_xcode/build"

EXCLUDES_ALL=""
for excl in $EXCLUDES; do
    EXCLUDES_ALL="$EXCLUDES_ALL --exclude=$excl"
done

RSYNC_OPTIONS="-vprt --delete $EXCLUDES_ALL"

echo [xcode-build-remote] Synching remote project at $REMOTE_HOST:$REMOTE_PATH
# echo rsync $RSYNC_OPTIONS -e ssh $LOCAL_PATH $REMOTE_USER@$REMOTE_HOST:$REMOTE_PATH
rsync $RSYNC_OPTIONS -e ssh $LOCAL_PATH $REMOTE_USER@$REMOTE_HOST:$REMOTE_PATH
if [ $? -ne 0 ]; then
    echo [xcode-build-remote] rsync failed!
    exit
fi

BUILDCMD=\
"echo [xcode-build-remote] Building...; "\
"cd $REMOTE_PATH/builds/mobicast/proj_xcode; "\
"xcodebuild -config Debug CODE_SIGN_IDENTITY=\"\" CODE_SIGNING_REQUIRED=\"NO\""

ssh $REMOTE_USER@$REMOTE_HOST $BUILDCMD
if [ $? -ne 0 ]; then
    echo [xcode-build-remote] Build failed!
    exit
fi

echo [xcode-build-remote] Build completed successfully!

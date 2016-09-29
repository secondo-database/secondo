#!/bin/bash
cd $HOME/Documents/EclipseWorkspaces/SECONDO/secondo
export SECONDO_SDK=$HOME/secondo-sdk
export SECONDO_PLATFORM=mac_osx
export SECONDO_BUILD_DIR=$HOME/Documents/EclipseWorkspaces/SECONDO/secondo
source $SECONDO_SDK/secondorc
export BDB_LIB_DIR=$SECONDO_SDK/lib
export LIBRARY_PATH=$LIBRARY_PATH:/usr/local/lib
export SECONDO_ADDITIONAL_DEFAULTCCFLAGS="-I /usr/local/include"
export JNI_H_DIR=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk/System/Library/Frameworks/JavaVM.framework/Headers/

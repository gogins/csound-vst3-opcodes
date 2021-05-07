#!/bin/bash
echo "Making a clean build of csound-vst3..."
rm -rf ./build-macos
mkdir -p build-macos
cd build-macos
cmake ../vst3sdk  -DCMAKE_CXX_FLAGS=-I/usr/local/include -DCMAKE_EXE_LINKER_FLAGS=/Library/Frameworks/CsoundLib64.framework/CsoundLib64 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS=-O2 -DSMTG_MYPLUGINS_SRC_PATH=../csound-vst3 -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 -GXcode
xcodebuild
find . -name "*vst3_plugins*" -ls
echo "Completed a clean build of csound-vst3."

# The last stage is to copy the plugin to Csound's plugin directory, and
# correct its name. We used
#
# cp csound-vst3-opcodes/build/lib/Debug/libvst3_plugins.so Frameworks/CsoundLib64.framework/Versions/6.0/Resources/Opcodes64/libvst3_plugins.dylib
#
#This location may differ on your system.


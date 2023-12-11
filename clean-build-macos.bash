#!/bin/bash
clear
echo "Making a clean build of csound-vst3..."
find . -name CMakeCache.txt -delete
rm -rf build-macos
mkdir -p build-macos
cd vst3sdk
echo "Patching the VST3 SDK to also build csound-vst-opcodes..."
git apply ../patches/vst3sdk.patch
cd ..
cd build-macos
cmake ../vst3sdk -DCMAKE_BUILD_TYPE=RelWithDebInfo -DSMTG_BUILD_UNIVERSAL_BINARY=1 -DCMAKE_C_FLAGS=-O2 -DCMAKE_CXX_FLAGS="-O2" -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 -GXcode

xcodebuild

mv lib/Debug/libvst3_plugins.so lib/Debug/libvst3_plugins.dylib
cd ..
find . -name "libvst3_plugins.*" -ls 2>/dev/null
echo "Completed a clean build of csound-vst3."


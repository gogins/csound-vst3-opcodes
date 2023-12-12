#!/bin/bash
clear
echo "Starting a clean build of csound-vst3 for macOS..."
find . -name CMakeCache.txt -delete
rm -rf build-macos
mkdir -p build-macos
cd build-macos
cmake ../vst3-opcodes -DCMAKE_BUILD_TYPE=RelWithDebInfo -DSMTG_BUILD_UNIVERSAL_BINARY=1 -DCMAKE_C_FLAGS=-O2 -DCMAKE_CXX_FLAGS=-O2 -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 -GXcode
xcodebuild
mv lib/Debug/libvst3_plugins.so lib/Debug/libvst3_plugins.dylib
cd ..
find . -name "libvst3_plugins.*" -ls 2>/dev/null
echo "Completed a clean build of csound-vst3 for macOS."


#!/bin/bash
echo "Making a clean build of csound-vst3..."
rm -rf ./build-macos
mkdir -p build-macos
cd build-macos
cmake ../vst3sdk  -DCMAKE_CXX_FLAGS=-I/usr/local/include -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS=-O2 -DSMTG_MYPLUGINS_SRC_PATH=../csound-vst3 -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 -GXcode
xcodebuild
mv lib/Debug/libvst3_plugins.so lib/Debug/libvst3_plugins.dylib
find . -type f -name "*.so" -exec cp {} build-macos/ \;
find . -type f -name "*.dylib" -exec cp {} build-macos/ \;
echo "Built plugins:"
find build-macos/ \( -name "*.dylib" -o -name "*.so" \) -ls
echo "Completed a clean build of csound-vst3."
# Install to: Frameworks/CsoundLib64.framework/Versions/6.0/Resources/Opcodes64/libvst3_plugins.dylib
# This location may differ on your system.


#!/bin/bash
echo "Making a clean build of csound-vst3..."
rm -rf build-macos
mkdir -p build-macos
export MYPLUGINS_PATH=$(pwd)/csound-vst3
echo "MYPLUGINS_PATH: ${MYPLUGINS_PATH}"
cd build-macos
cmake ../vst3sdk -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS=-O2 -DCMAKE_CXX_FLAGS=-O2 -DSMTG_MYPLUGINS_SRC_PATH:PATH=${MYPLUGINS_PATH} -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 -GXcode
xcodebuild 
ls -ll lib/Debug
mv lib/Debug/libvst3_plugins.so lib/Debug/libvst3_plugins.dylib
cd ..
echo "Completed a clean build of csound-vst3."
echo "Install to: Frameworks/CsoundLib64.framework/Versions/6.0/Resources/Opcodes64/libvst3_plugins.dylib."
echo "This location may differ on your system."


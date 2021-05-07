#!/bin/bash
echo "Making a clean build of csound-vst3..."
rm -rf ./build-windows
mkdir -p build-windows
cd build-windows
cmake ../vst3sdk -DCMAKE_BUILD_TYPE=RelWithDebug -DSMTG_MYPLUGINS_SRC_PATH=../csound-vst3
dir
cmake --build .
dir
dir x64
dir lib
find . -name "*vst3_plugins*" -ls
echo "Completed a clean build of csound-vst3..."


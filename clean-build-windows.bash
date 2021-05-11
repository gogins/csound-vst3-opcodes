#!/bin/bash
echo "Making a clean build of csound-vst3..."
rm -rf ./build-windows
mkdir -p build-windows
cd build-windows
cmake ../vst3sdk -DCMAKE_BUILD_TYPE=RelWithDebug -DSMTG_CREATE_PLUGIN_LINK=0 -DSMTG_MYPLUGINS_SRC_PATH=../csound-vst3
dir
cmake --build .
echo "Built:"
find . -name "*vst3_plugins*" -ls
cd ..
echo "Completed a clean build of csound-vst3."


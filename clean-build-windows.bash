#!/bin/bash
echo "Making a clean build of csound-vst3..."
rm -rf ./build-windows
mkdir -p build-windows
cd build-windows
cmake ../vst3sdk -DCMAKE_BUILD_TYPE=RelWithDebug -DSMTG_CREATE_PLUGIN_LINK=0 -DSMTG_MYPLUGINS_SRC_PATH:PATH="../csound-vst3"
cmake --build .
cd ..
echo "Completed a clean build of csound-vst3."


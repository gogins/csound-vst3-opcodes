#!/bin/bash
echo "Making a clean build of csound-vst3..."
rm -rf ./build-windows
mkdir -p build-windows
cd build-windows
cmake ../vst3sdk -DCMAKE_BUILD_TYPE=RelWithDebug -DSMTG_CREATE_PLUGIN_LINK=0 -DSMTG_MYPLUGINS_SRC_PATH=../csound-vst3
dir
cmake --build .
find . -type f -name "*.DLL" -exec cp {} . \;
find . -type f -name "*.dll" -exec cp {} . \;
find . -type f -name "*.pdb" -exec cp {} . \;
find . -name "*.vst3" -exec cp -rf {} . \;
echo "Built plugins:"
pwd
ls -ll
echo "Built plugins:"
pwd
ls -ll 
cd ..
echo "Completed a clean build of csound-vst3."


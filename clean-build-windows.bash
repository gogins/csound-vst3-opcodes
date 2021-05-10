#!/bin/bash
echo "Making a clean build of csound-vst3..."
rm -rf ./build-windows
mkdir -p build-windows
cd build-windows
cmake ../vst3sdk -DCMAKE_BUILD_TYPE=RelWithDebug -DSMTG_MYPLUGINS_SRC_PATH=../csound-vst3
dir
cmake --build .
find . -type f -name "*.dll" -exec cp {} . \;
find . -type f -name "*.pdb" -exec cp {} . \;
echo "Built plugins:"
ls -ll *.dll *.pdb
echo "Completed a clean build of csound-vst3."


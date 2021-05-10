#!/bin/bash
echo "Making a clean build of csound-vst3..."
rm -rf ./build-linux
mkdir -p build-linux
cd build-linux
cmake ../vst3sdk -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS=-O2 -DCMAKE_CXX_FLAGS=-O2 -DSMTG_MYPLUGINS_SRC_PATH=../csound-vst3
make clean
make VERBOSE=1 j=4
find . -type f -name "*.so" -exec cp {} . \;
echo "Built plugins:"
pwd
ls -ll *.so
echo "Completed a clean build of csound-vst3."

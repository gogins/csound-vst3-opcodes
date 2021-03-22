#!/bin/bash
echo "Making a clean build of csound-vst3 for debugging with optimization..."
rm -rf ./build
mkdir -p build
cd build
cmake ../vst3sdk -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS=-O2 -DCMAKE_CXX_FLAGS=-O2 -DSMTG_MYPLUGINS_SRC_PATH=../csound-vst3
make clean
make VERBOSE=1 j=4
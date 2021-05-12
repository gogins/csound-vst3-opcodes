#!/bin/bash
echo "Making a clean build of csound-vst3..."
rm -rf build-windows
mkdir -p build-windows
export MYPLUGINS_PATH=$(pwd)/csound-vst3
echo "MYPLUGINS_PATH: ${MYPLUGINS_PATH}"
cd build-windows
cmake ../vst3sdk -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS=-O2 -DCMAKE_CXX_FLAGS=-O2 -DSMTG_MYPLUGINS_SRC_PATH:PATH=${MYPLUGINS_PATH}
cmake --build .
cd ..
echo "Completed a clean build of csound-vst3."


#!/bin/bash
echo "Making a clean build of csound-vst3..."
rm -rf build-linux
mkdir -p build-linux
cd vst3sdk
echo "Patching the VST3 SDK to also build csound-vst-opcodes..."
git apply ../patches/vst3sdk.patch
cd ..
cd build-linux
ls ..
pwd
ls ../csound-vst3
cmake ../vst3sdk -DCMAKE_CONFIGURATION_TYPES="Release;" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS=-O2 -DCMAKE_CXX_FLAGS="-O2 -std=c++17" -DSMTG_MYPLUGINS_SRC_PATH=../csound-vst3
cmake --build . --config Release --verbose --parallel 4
cd ..
echo "Completed a clean build of csound-vst3."

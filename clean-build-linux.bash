#!/bin/bash
echo "Making a clean build of csound-vst3..."
rm -rf build-linux
mkdir -p build-linux
# Locally it is:
# ~/csound-vst3-opcodes/csound-vst3/
# On the runner it is constructed as:
# /home/runner/work/csound-vst3-opcodes/csound-vst3-opcodes/csound-vst3
export MYPLUGINS_PATH=$(pwd)/csound-vst3
echo "MYPLUGINS_PATH: $MYPLUGINS_PATH"
cd build-linux
ls ..
pwd
ls ../csound-vst3
cmake ../vst3sdk -DCMAKE_CONFIGURATION_TYPES="Release;" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS=-O2 -DCMAKE_CXX_FLAGS=-O2 -DSMTG_MYPLUGINS_SRC_PATH=../csound-vst3
cmake --build . --config Release --verbose --parallel 4
cd ..
echo "Completed a clean build of csound-vst3."

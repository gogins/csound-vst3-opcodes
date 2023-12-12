#!/bin/bash
clear
echo "Starting a clean build of csound-vst3 for Linux..."
find . -name CMakeCache.txt -delete
rm -rf build-linux
mkdir -p build-linux
cd build-linux
cmake ../vst3-opcodes -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_FLAGS=-O2 -DCMAKE_CXX_FLAGS=-O2
cmake --build . --verbose --parallel 4
cd ..
find . -name "libvst3_plugins.*" -ls 2>/dev/null
echo "Completed a clean build of csound-vst3 for Linux."


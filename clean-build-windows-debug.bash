#!/bin/bash
echo "Starting a clean Debug build of csound-vst3 for Windows..."
rm -rf build-windows
mkdir -p build-windows
cd build-windows
cmake ../vst3-opcodes -DCMAKE_BUILD_TYPE=Debug -DSMTG_USE_STDATOMIC_H=OFF
cmake --build . --config Debug --verbose --parallel 4
cd ..
echo "Completed a clean Debug build of csound-vst3 for Windows."
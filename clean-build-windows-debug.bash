#!/bin/bash
echo "Making a clean build of csound-vst3..."
rm -rf build-windows-debug
mkdir -p build-windows-debug
cd vst3sdk
echo "Patching the VST3 SDK to also build csound-vst-opcodes..."
git apply ../patches/vst3sdk.patch
cd ..
cd build-windows-debug
cmake ../vst3sdk -DCMAKE_BUILD_TYPE=Debug -DSMTG_MYPLUGINS_SRC_PATH=../csound-vst3 -DSMTG_USE_STDATOMIC_H=OFF
cmake --build . --config Debug --verbose --parallel 4
cd ..
echo "Completed a clean build of csound-vst3."


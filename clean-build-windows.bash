#!/bin/bash
echo "Making a clean build of csound-vst3..."
rm -rf build-windows
mkdir -p build-windows
cd build-windows
cmake ../vst3sdk -DCMAKE_BUILD_TYPE=Release -DSMTG_MYPLUGINS_SRC_PATH=../csound-vst3 -DSMTG_USE_STDATOMIC_H=OFF
cmake --build . --config Release --verbose --parallel 4
cd ..
echo "Completed a clean build of csound-vst3."


#!/bin/bash
clear
echo "Starting a clean build of csound-vst3-opcodes for macOS..."
find . -name CMakeCache.txt -delete
rm -rf build-macos
mkdir -p build-macos
cd build-macos
# A lot of patching is needed here, I wonder why.
# A lot of patching is needed here, I wonder why.
cmake ../vst3-opcodes \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DSMTG_BUILD_UNIVERSAL_BINARY=1 \
    -DCMAKE_C_FLAGS="-O2" \
    -DCMAKE_CXX_FLAGS="-Wno-error=unused-but-set-variable -Wno-error=deprecated-declarations" \
    -DCMAKE_SHARED_LINKER_FLAGS="-framework Foundation" \
    -DCMAKE_EXE_LINKER_FLAGS="-framework Foundation" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 \
    -G "Unix Makefiles"
    
cmake --build . --verbose --parallel 4
mv lib/RelWithDebInfo/libvst3_plugins.so lib/RelWithDebInfo/libvst3_plugins.dylib
cd ..
find . -name "libvst3_plugins.*" -ls 2>/dev/null
echo "Completed a clean build of csound-vst3-opcodes for macOS."


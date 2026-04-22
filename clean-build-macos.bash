#!/bin/bash
set -euo pipefail

clear
echo "Starting a clean build of csound-vst3-opcodes for macOS..."

find . -name CMakeCache.txt -delete 2>/dev/null || true
rm -rf build-macos
mkdir -p build-macos
cd build-macos

CSOUND_FRAMEWORK_DIR="/Library/Frameworks"
CSOUND_FRAMEWORK="$CSOUND_FRAMEWORK_DIR/CsoundLib64.framework"
CSOUND_INCLUDE_DIR="$CSOUND_FRAMEWORK/Headers"
CSOUND_LIBRARY="$CSOUND_FRAMEWORK/CsoundLib64"
CSOUND_PLUGIN_DIR="$HOME/Library/csound/7.0/plugins64"

cmake ../vst3-opcodes \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DSMTG_BUILD_UNIVERSAL_BINARY=0 \
    -DCMAKE_C_FLAGS="-O2" \
    -DCMAKE_CXX_FLAGS="-Wno-error=unused-but-set-variable -Wno-error=deprecated-declarations" \
    -DCMAKE_SHARED_LINKER_FLAGS="-framework Foundation -F$CSOUND_FRAMEWORK_DIR -Wl,-rpath,$CSOUND_FRAMEWORK_DIR" \
    -DCMAKE_EXE_LINKER_FLAGS="-framework Foundation -F$CSOUND_FRAMEWORK_DIR -Wl,-rpath,$CSOUND_FRAMEWORK_DIR" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 \
    -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DCMAKE_PREFIX_PATH="/Library;/Library/Frameworks" \
    -DCMAKE_FRAMEWORK_PATH="$CSOUND_FRAMEWORK_DIR" \
    -DCSOUND_INCLUDE_DIR="$CSOUND_INCLUDE_DIR" \
    -DCSOUND_INCLUDE_DIRS="$CSOUND_INCLUDE_DIR" \
    -DCSOUND_LIBRARY="$CSOUND_LIBRARY" \
    -DCSOUND_LIBRARIES="$CSOUND_LIBRARY" \
    -G "Unix Makefiles"

cmake --build . --verbose --parallel 4 --target vst3_plugins

# Builds here: $HOME/csound-vst3-opcodes/build-macos/lib/RelWithDebInfo/libvst3_plugins.so
BUILT_SO="lib/RelWithDebInfo/libvst3_plugins.so"
BUILT_DYLIB="lib/RelWithDebInfo/libvst3_plugins.dylib"

if [ -f "$BUILT_DYLIB" ]; then
    rm -f "$BUILT_DYLIB"
fi

cp -f "$BUILT_SO" "$BUILT_DYLIB"

mkdir -p "$CSOUND_PLUGIN_DIR"
cp -f "$BUILT_DYLIB" "$CSOUND_PLUGIN_DIR/"

cd ..
find ~/ -name "libvst3_plugins.*" -ls 2>/dev/null

echo "Installed libvst3_plugins.dylib to:"
echo "  $CSOUND_PLUGIN_DIR"
echo "Completed a clean build of csound-vst3-opcodes for macOS."
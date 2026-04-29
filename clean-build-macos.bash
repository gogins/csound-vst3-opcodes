#!/usr/bin/env bash
set -euo pipefail
echo "Cleaning and building csound-vst3-opcodes for macOS..."
repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
build_dir="$repo_root/build-macos"

rm -rf "$build_dir" "$repo_root/dist"
cmake -S "$repo_root/vst3-opcodes" -B "$build_dir" -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DSMTG_BUILD_UNIVERSAL_BINARY=OFF \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 \
    -DCMAKE_OSX_ARCHITECTURES="arm64" \
    -DCMAKE_PREFIX_PATH="/Library;/Library/Frameworks" \
    -DCMAKE_FRAMEWORK_PATH="/Library/Frameworks" \
    -DCMAKE_CXX_FLAGS="-Wno-error=unused-but-set-variable -Wno-error=deprecated-declarations"
cmake --build "$build_dir" --parallel 4 --target stage_dist
cmake --build "$build_dir" --target sign_dist
cmake --build "$build_dir" --target archive_dist
cmake --build "$build_dir" --target notarize_dist
echo "Completed clean build of csound-vst3-opcodes for macOS. Built artifacts are in dist/."    

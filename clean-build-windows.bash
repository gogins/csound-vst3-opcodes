#!/usr/bin/env bash
set -euo pipefail
echo "Cleaning and building csound-vst3-opcodes for Windows..."
repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
build_dir="$repo_root/build-windows"
config="${1:-Release}"

rm -rf "$build_dir" "$repo_root/dist"
cmake -S "$repo_root/vst3-opcodes" -B "$build_dir" \
    -DCMAKE_BUILD_TYPE="$config" \
    -DSMTG_USE_STDATOMIC_H=OFF
cmake --build "$build_dir" --config "$config" --parallel 4 --target stage_dist
cmake --build "$build_dir" --config "$config" --target archive_dist
echo "Completed clean build of csound-vst3-opcodes for Windows. Built artifacts are in dist/."

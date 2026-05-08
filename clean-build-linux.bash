#!/usr/bin/env bash
set -euo pipefail

echo "Cleaning and building csound-vst3-opcodes for Linux..."
repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
build_dir="${repo_root}/build-linux"
install_dir="${repo_root}/dist"
cmake_args=("$@")

rm -rf "${build_dir}" "${install_dir}"
cmake -S "${repo_root}/vst3-opcodes" -B "${build_dir}" -G Ninja \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_INSTALL_PREFIX="${install_dir}" \
    "${cmake_args[@]}"
cmake --build "${build_dir}" --parallel --target stage_dist
cmake --install "${build_dir}" --prefix "${install_dir}"
cmake --build "${build_dir}" --target archive_dist
find "${install_dir}" -type f -print
cmake -E echo "Archive: ${build_dir}/csound-vst3-opcodes-2.0.0-linux.zip"
echo "Completed clean build of csound-vst3-opcodes for Linux. Built artifacts are in dist/."

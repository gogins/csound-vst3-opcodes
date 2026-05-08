#!/usr/bin/env bash
set -euo pipefail

echo "Cleaning and building csound-vst3-opcodes for Windows..."
repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
build_dir="${repo_root}/build-windows"
install_dir="${repo_root}/dist"
config="${1:-Release}"
if [[ $# -gt 0 ]]; then
    shift
fi
cmake_args=("$@")

rm -rf "${build_dir}" "${install_dir}"
cmake -S "${repo_root}/vst3-opcodes" -B "${build_dir}" \
    -DCMAKE_BUILD_TYPE="${config}" \
    -DCMAKE_INSTALL_PREFIX="${install_dir}" \
    -DSMTG_USE_STDATOMIC_H=OFF \
    "${cmake_args[@]}"
cmake --build "${build_dir}" --config "${config}" --parallel --target stage_dist
cmake --install "${build_dir}" --config "${config}" --prefix "${install_dir}"
cmake --build "${build_dir}" --config "${config}" --target archive_dist
find "${install_dir}" -type f -print
cmake -E echo "Archive: ${build_dir}/csound-vst3-opcodes-2.0.0-windows.zip"
echo "Completed clean build of csound-vst3-opcodes for Windows. Built artifacts are in dist/."

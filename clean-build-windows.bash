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
launcher_args=()

if command -v ccache >/dev/null 2>&1
then
    launcher_args=(
        -DCMAKE_C_COMPILER_LAUNCHER=ccache
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
    )
fi

rm -rf "${build_dir}" "${install_dir}"
cmake -S "${repo_root}/vst3-opcodes" -B "${build_dir}" -G Ninja \
    -DCMAKE_BUILD_TYPE="${config}" \
    -DCMAKE_INSTALL_PREFIX="${install_dir}" \
    -DSMTG_USE_STDATOMIC_H=OFF \
    "${launcher_args[@]}" \
    "${cmake_args[@]}"

cmake --build "${build_dir}" --parallel --target stage_dist
cmake --build "${build_dir}" --target archive_dist
find "${install_dir}" -type f -print
cmake -E echo "Archive: ${build_dir}/csound-vst3-opcodes-2.0.0-windows.zip"
echo "Completed clean build of csound-vst3-opcodes for Windows. Built artifacts are in dist/."

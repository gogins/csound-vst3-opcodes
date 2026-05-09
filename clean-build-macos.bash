#!/usr/bin/env bash
set -euo pipefail

echo "Cleaning and building csound-vst3-opcodes for macOS..."

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
build_dir="${repo_root}/build-macos"
install_dir="${repo_root}/dist"

csound_ac_enable_codesign="OFF"
csound_ac_enable_notarization="OFF"
cmake_args=()

for arg in "$@"
do
    case "${arg}" in
        -DCSOUND_AC_ENABLE_CODESIGN=ON)
            csound_ac_enable_codesign="ON"
            ;;
        -DCSOUND_AC_ENABLE_NOTARIZATION=ON)
            csound_ac_enable_notarization="ON"
            ;;
    esac
    cmake_args+=("${arg}")
done

rm -rf "${build_dir}" "${install_dir}"
cmake -S "${repo_root}/vst3-opcodes" -B "${build_dir}" -G Ninja \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DSMTG_BUILD_UNIVERSAL_BINARY=OFF \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 \
    -DCMAKE_OSX_ARCHITECTURES="${CMAKE_OSX_ARCHITECTURES:-arm64}" \
    -DCMAKE_INSTALL_PREFIX="${install_dir}" \
    -DCMAKE_PREFIX_PATH="/Library;/Library/Frameworks;/opt/homebrew;/usr/local" \
    -DCMAKE_FRAMEWORK_PATH="/Library/Frameworks;$HOME/Library/Frameworks" \
    -DCMAKE_CXX_FLAGS="-Wno-error=unused-but-set-variable -Wno-error=deprecated-declarations" \
    ${cmake_args[@]+"${cmake_args[@]}"}

cmake --build "${build_dir}" --parallel --target stage_dist
cmake --install "${build_dir}" --prefix "${install_dir}"

if [[ "${csound_ac_enable_codesign}" == "ON" && "${csound_ac_enable_notarization}" == "ON" ]]
then
    cmake --build "${build_dir}" --target release_dist
else
    cmake --build "${build_dir}" --target release_dist
fi

find "${install_dir}" -type f -print
cmake -E echo "Archive: ${build_dir}/csound-vst3-opcodes-2.0.0-darwin.zip"
echo "Completed clean build of csound-vst3-opcodes for macOS. Built artifacts are in dist/."

#!/usr/bin/env bash
set -euo pipefail

echo "Cleaning and building csound-vst3-opcodes for macOS..."

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
build_dir="${repo_root}/build-macos"
install_dir="${repo_root}/dist"
archive_path="${build_dir}/csound-vst3-opcodes-2.0.0-darwin.zip"
codesign_check_script="${repo_root}/codesign-check.bash"

csound_vst3_opcode_sign="OFF"
csound_vst3_opcode_notarize="OFF"
cmake_args=()
launcher_args=()

if command -v ccache >/dev/null 2>&1
then
    launcher_args=(
        -DCMAKE_C_COMPILER_LAUNCHER=ccache
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
    )
fi

if [[ "${GITHUB_ACTIONS:-}" == "true" ]]
then
    csound_vst3_opcode_sign="ON"
    csound_vst3_opcode_notarize="ON"
    cmake_args+=(
        -DCSOUND_VST3_OPCODE_SIGN=ON
        -DCSOUND_VST3_OPCODE_NOTARIZE=ON
    )
fi

for arg in "$@"
do
    case "${arg}" in
        -DCSOUND_VST3_OPCODE_SIGN=ON|-DCSOUND_AC_ENABLE_CODESIGN=ON)
            csound_vst3_opcode_sign="ON"
            ;;
        -DCSOUND_VST3_OPCODE_NOTARIZE=ON|-DCSOUND_AC_ENABLE_NOTARIZATION=ON)
            csound_vst3_opcode_notarize="ON"
            ;;
    esac
    cmake_args+=("${arg}")
done

if [[ "${csound_vst3_opcode_sign}" == "ON" && -z "${APPLE_CODESIGN_IDENTITY:-}" ]]
then
    echo "ERROR: Code signing is enabled but APPLE_CODESIGN_IDENTITY is not set." >&2
    exit 1
fi

if [[ "${csound_vst3_opcode_notarize}" == "ON" ]]
then
    if [[ -z "${APPLE_NOTARY_KEY:-}" || -z "${APPLE_NOTARY_KEY_ID:-}" || -z "${APPLE_NOTARY_ISSUER_ID:-}" ]]
    then
        echo "ERROR: Notarization is enabled but APPLE_NOTARY_KEY, APPLE_NOTARY_KEY_ID, or APPLE_NOTARY_ISSUER_ID is not set." >&2
        exit 1
    fi
fi

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
    ${launcher_args[@]+"${launcher_args[@]}"} \
    ${cmake_args[@]+"${cmake_args[@]}"}

cmake --build "${build_dir}" --parallel --target stage_dist
cmake --build "${build_dir}" --target release_dist

if [[ "${csound_vst3_opcode_sign}" == "ON" && "${csound_vst3_opcode_notarize}" == "ON" ]]
then
    if [[ ! -x "${codesign_check_script}" ]]
    then
        echo "ERROR: codesign check script not found or not executable: ${codesign_check_script}" >&2
        exit 1
    fi
    "${codesign_check_script}" "${archive_path}"
fi

find "${install_dir}" -type f -print
cmake -E echo "Archive: ${archive_path}"

echo "Completed clean build of csound-vst3-opcodes for macOS. Built artifacts are in dist/."

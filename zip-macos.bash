#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
build_dir="${repo_root}/build-macos"
cmake --build "${build_dir}" --target archive_dist
cmake -E tar tf "${build_dir}/csound-vst3-opcodes-2.0.0-darwin.zip"

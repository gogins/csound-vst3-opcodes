#!/usr/bin/env bash
set -euo pipefail

dist_bin_dir="$1"
sign_enabled="$2"
codesign_identity="$3"

if [[ "$sign_enabled" != "ON" || -z "$codesign_identity" ]]; then
    echo "Skipping macOS codesign."
    exit 0
fi

find "$dist_bin_dir" -type f \( -perm -111 -o -name "*.dylib" -o -name "*.so" \) -print0 |
while IFS= read -r -d '' artifact; do
    codesign --force --timestamp --options runtime --sign "$codesign_identity" "$artifact"
done
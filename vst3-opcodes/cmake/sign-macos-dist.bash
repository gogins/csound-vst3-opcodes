#!/usr/bin/env bash
set -euo pipefail

dist_dir="${1:?dist directory is required}"
enable_codesign="${2:-OFF}"
codesign_identity="${APPLE_CODESIGN_IDENTITY:-}"

case "${enable_codesign}" in
    ON|on|On|TRUE|true|True|1|YES|yes|Yes)
        ;;
    *)
        echo "Code signing disabled."
        exit 0
        ;;
esac

if [[ -z "${codesign_identity}" ]]; then
    echo "APPLE_CODESIGN_IDENTITY is not set; skipping signing."
    exit 0
fi

if ! command -v codesign >/dev/null 2>&1; then
    echo "codesign not found; skipping signing."
    exit 0
fi

while IFS= read -r -d '' artifact
do
    if file "${artifact}" | grep -q "Mach-O"; then
        echo "Signing ${artifact}"
        codesign --force --timestamp --options runtime --sign "${codesign_identity}" "${artifact}"
    fi
done < <(find "${dist_dir}" -type f -print0)

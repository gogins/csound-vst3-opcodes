#!/usr/bin/env bash
set -euo pipefail

archive_path="${1:?archive path is required}"
enable_notarization="${2:-OFF}"
notary_key="${APPLE_NOTARY_KEY:-}"
notary_key_id="${APPLE_NOTARY_KEY_ID:-}"
notary_issuer_id="${APPLE_NOTARY_ISSUER_ID:-}"

case "${enable_notarization}" in
    ON|on|On|TRUE|true|True|1|YES|yes|Yes)
        ;;
    *)
        echo "Notarization disabled."
        exit 0
        ;;
esac

if [[ -z "${notary_key}" || -z "${notary_key_id}" || -z "${notary_issuer_id}" ]]; then
    echo "ERROR: APPLE_NOTARY_KEY, APPLE_NOTARY_KEY_ID, or APPLE_NOTARY_ISSUER_ID is not set, but notarization is enabled." >&2
    exit 1
fi

if [[ ! -f "${archive_path}" ]]; then
    echo "Archive not found: ${archive_path}" >&2
    exit 1
fi

xcrun notarytool submit "${archive_path}" \
    --key "${notary_key}" \
    --key-id "${notary_key_id}" \
    --issuer "${notary_issuer_id}" \
    --wait

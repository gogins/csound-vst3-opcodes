#!/usr/bin/env bash
set -euo pipefail

archive="$1"
notarize_enabled="$2"
key="$3"
key_id="$4"
issuer="$5"

if [[ "$notarize_enabled" != "ON" ]]; then
    echo "Skipping notarization: CSOUND_VST3_OPCODE_NOTARIZE is OFF."
    exit 0
fi

if [[ -z "$archive" ]]; then
    echo "Archive path was not supplied." >&2
    exit 1
fi

if [[ -z "$key" || -z "$key_id" || -z "$issuer" ]]; then
    echo "Skipping notarization: API key credentials were not supplied."
    exit 0
fi

if [[ ! -f "$archive" ]]; then
    echo "Archive does not exist: $archive" >&2
    exit 1
fi

xcrun notarytool submit "$archive" \
    --key "$key" \
    --key-id "$key_id" \
    --issuer "$issuer" \
    --wait || { echo "Notarization failed for $archive" >&2; exit 1; }

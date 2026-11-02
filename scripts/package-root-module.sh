#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
output_dir="$repo_root/build/releases"
mkdir -p "$output_dir"
archive="$output_dir/soniccore-root-0.1.0.zip"
rm -f "$archive"
(
    cd "$repo_root/root-module"
    zip -qr "$archive" .
)
echo "$archive"


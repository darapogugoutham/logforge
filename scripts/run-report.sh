#!/usr/bin/env bash
# Usage: run-report.sh INPUT OUTPUT [THREADS]
# flock prevents overlapping scheduled runs. Rename publishes a complete report.
set -euo pipefail
[[ $# -ge 2 && $# -le 3 ]] || { echo "Usage: $0 INPUT OUTPUT [THREADS]" >&2; exit 2; }
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
input=$1
output=$2
mkdir -p "$(dirname "$output")"
exec 9>"${output}.lock"
flock -n 9 || { echo 'Another report is running' >&2; exit 75; }
temporary=$(mktemp "${output}.XXXXXX")
trap 'rm -f "$temporary"' EXIT
"$root/build/logforge" --input "$input" --threads "${3:-4}" > "$temporary"
mv -f "$temporary" "$output"
echo "Report published: $output"

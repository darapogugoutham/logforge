#!/usr/bin/env bash
# Exit 0 healthy, 1 threshold exceeded, 2 bad/missing report.
set -euo pipefail
[[ $# -ge 1 && $# -le 2 ]] || { echo "Usage: $0 REPORT [MAX_ERROR_PERCENT]" >&2; exit 2; }
python3 - "$1" "${2:-5}" <<'PY'
import json, sys
try:
    with open(sys.argv[1]) as stream:
        report = json.load(stream)
    threshold = float(sys.argv[2])
    if not 0 <= threshold <= 100:
        raise ValueError('threshold must be 0..100')
    valid = report['valid']
    errors = report['levels'].get('ERROR', 0)
    rate = 100 * errors / valid if valid else 0
    print(f"valid={valid} errors={errors} error_percent={rate:.2f} malformed={report['malformed']}")
    sys.exit(1 if rate > threshold else 0)
except (OSError, ValueError, KeyError, TypeError) as error:
    print(f'monitor: {error}', file=sys.stderr)
    sys.exit(2)
PY

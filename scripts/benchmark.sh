#!/usr/bin/env bash
# Generates synthetic logs, verifies identical counts, writes measured timings.
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")/.."
./scripts/build.sh
python3 - <<'PY'
import json, pathlib, platform, subprocess, time
root = pathlib.Path('benchmark-data')
root.mkdir(exist_ok=True)
line = '2026-10-01T12:00:00Z INFO request completed successfully\n'
for i in range(64):
    (root / f'{i}.log').write_text(line * 10000)
results = []
for workers in (1, 2, 4, 8):
    timings = []
    for trial in range(3):
        start = time.perf_counter()
        output = subprocess.check_output(['build/logforge', '--input', str(root), '--threads', str(workers)])
        elapsed = time.perf_counter() - start
        report = json.loads(output)
        assert report == dict(files=64, lines=640000, valid=640000, malformed=0, levels={'INFO':640000})
        timings.append(elapsed)
    results.append(dict(threads=workers, seconds=timings, best_seconds=min(timings)))
pathlib.Path('reports').mkdir(exist_ok=True)
payload = dict(platform=platform.platform(), records=640000, files=64, trials=3, results=results)
pathlib.Path('reports/benchmark.json').write_text(json.dumps(payload, indent=2) + '\n')
print(json.dumps(payload, indent=2))
PY

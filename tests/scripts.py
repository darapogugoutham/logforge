"""Verify operations behavior without installing a Linux service."""
import fcntl
import json
import pathlib
import subprocess
import tempfile

root = pathlib.Path(__file__).resolve().parents[1]
def run(*args):
    return subprocess.run(['bash', *map(str, args)], cwd=root, timeout=20)

for script in (root / 'scripts').glob('*.sh'):
    assert run('-n', script).returncode == 0
with tempfile.TemporaryDirectory() as directory:
    output = pathlib.Path(directory) / 'space dir/report.json'
    assert run('scripts/run-report.sh', 'examples', output, 2).returncode == 0
    assert json.loads(output.read_text())['lines'] == 4
    assert run('scripts/monitor.sh', output, 30).returncode == 1
    assert run('scripts/monitor.sh', output, 50).returncode == 0
    assert run('scripts/monitor.sh', output, 'bad').returncode == 2
    previous = output.read_bytes()
    assert run('scripts/run-report.sh', 'missing', output, 2).returncode != 0
    assert output.read_bytes() == previous
    with open(str(output) + '.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        assert run('scripts/run-report.sh', 'examples', output).returncode == 75
print('PASS Bash syntax, spaced paths, monitor statuses, report preservation, overlapping lock')

"""Independent expected counts and subprocess timeouts catch data loss/deadlocks."""
import json
import pathlib
import subprocess
import tempfile

binary = pathlib.Path(__file__).resolve().parents[1] / "build/logforge"
def run(*args):
    return subprocess.run([str(binary), *map(str, args)], capture_output=True, text=True, timeout=20)

with tempfile.TemporaryDirectory() as directory:
    root = pathlib.Path(directory)
    (root / "nested").mkdir()
    for i in range(40):
        (root / "nested" / f"{i}.log").write_text(
            "2026-10-01T12:00:00Z INFO ready\n" * 200
            + "2026-10-01T12:00:01Z ERROR failure\n" * 20
            + "broken\n" * 3)
    (root / "ignored.txt").write_text("broken\n")
    expected = dict(files=40, lines=8920, valid=8800, malformed=120,
                    levels={"ERROR": 800, "INFO": 8000})
    for threads in (1, 2, 8, 32):
        for capacity in (1, 16):
            result = run("--input", root, "--threads", threads, "--queue-capacity", capacity)
            assert result.returncode == 0, result.stderr
            assert json.loads(result.stdout) == expected
    for args in [("--input", root / "missing"), ("--input", root, "--threads", "0"),
                 ("--input", root, "--threads", "-1"), ("--input", root, "--threads", "257"),
                 ("--input", root, "--queue-capacity", "0"), ("--unknown", "x"), ("--input",)]:
        result = run(*args)
        assert result.returncode != 0 and not result.stdout
    empty = root / "empty"
    empty.mkdir()
    assert json.loads(run("--input", empty).stdout)["lines"] == 0
    one = run("--input", root / "nested/0.log", "--threads", 8)
    assert json.loads(one.stdout)["lines"] == 223
print("PASS 8 parallel configurations, malformed records, empty/single input, 7 CLI errors")

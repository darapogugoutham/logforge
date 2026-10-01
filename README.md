# LogForge

A C++17 parallel server-log analyzer with bounded work queues, Bash automation,
JSON reports, and scheduled Linux operation. Built as a personal portfolio project.

## Quick start

Requires Linux, GCC 9+ (or another C++17 compiler), GNU Make, Bash, Python 3,
and util-linux `flock`. There are no third-party C++ dependencies.

```bash
bash scripts/build.sh
./build/logforge --input examples --threads 4 --queue-capacity 64
bash scripts/test.sh
bash scripts/run-report.sh examples reports/latest.json 4
bash scripts/monitor.sh reports/latest.json 50
bash scripts/benchmark.sh
```

Input records use `YYYY-MM-DDTHH:MM:SSZ LEVEL message`, where LEVEL is INFO,
WARN, ERROR, or DEBUG. Directory input recursively includes regular `.log` files
and skips symlinks. A single-file input accepts any extension. Bad records are
counted separately. Timestamp shape and numeric characters are checked; this is
not a calendar-date validator. Reports are written to stdout; failures go to
stderr with a nonzero exit code.

Example report:

```json
{
  "files": 1,
  "lines": 4,
  "valid": 3,
  "malformed": 1,
  "levels": {"ERROR": 1, "INFO": 1, "WARN": 1}
}
```

## How threading works

The main thread discovers files and sends their paths to a bounded queue.
`std::thread` workers read independent files in parallel. A mutex protects queue
state; condition variables block producers when the queue is full and workers
when it is empty. Wait predicates handle spurious wakeups. Each worker owns its
statistics, and the main thread merges results only after joining all workers.
This avoids concurrent writes to a shared statistics map.

The queue provides backpressure instead of collecting all paths in memory.
Closing it wakes blocked producers and consumers; accepted work drains.
Worker errors close the queue, all threads join, and the first captured exception
is returned to the CLI. Reports are deterministic even if completion order varies.

This uses the C++ standard threading API. Linux builds link with `-pthread`;
there are no direct `pthread_create` calls and no Python worker threads.
Python is used only for verification, synthetic benchmarks, and report monitoring.

## Bash and Linux operations

| File | Purpose |
| --- | --- |
| `scripts/build.sh` | Repeatable compiler build with strict warnings |
| `scripts/test.sh` | Queue stress tests and CLI integration tests |
| `scripts/benchmark.sh` | Generate 640,000 records and compare 1/2/4/8 workers |
| `scripts/run-report.sh` | Prevent overlapping runs with flock and publish reports by rename |
| `scripts/monitor.sh` | Read a JSON report and return status from an error-rate threshold |
| `deploy/logforge.service` | Run analysis as an unprivileged Linux account |
| `deploy/logforge.timer` | Schedule analysis every five minutes |
| `.github/workflows/ci.yml` | Build, tests, Bash lint, and sanitizer jobs |

See [Linux runbook](docs/linux-runbook.md) for deployment and journal monitoring.
The scripts and service files are included; no remote server has been provisioned.

## Measured benchmark

An initial Linux container run processed 64 synthetic files / 640,000 valid records.
Each configuration ran three times and verified exact report counts. Best measured
times were 0.1365 s (1 thread), 0.0830 s (2), 0.0385 s (4), and 0.0284 s (8),
approximately 4.80x faster at 8 threads for this workload. See
[raw results](docs/benchmark-initial.json).

These are small, cache-sensitive synthetic measurements, not production latency
guarantees. Rerun the benchmark on your machine. Parallelism is across files:
one large file does not gain parsing parallelism, and more threads can hurt
performance on disk-bound workloads.

## Verification

`make test` checks 20,000 items sent by four concurrent producers to six consumers,
queue close/drain behavior, eight worker/capacity combinations, independent exact
counts, malformed input, single files, empty directories, and CLI failures.
Integration subprocess timeouts detect hangs. CI includes separate address/undefined
behavior and thread sanitizer checks. Local validation details are in
[validation.md](docs/validation.md); CI results must be checked on GitHub after push.

## Scope and limitations

This is batch analysis, not live `tail -f`, a network daemon, or durable job storage.
Inputs should be stable snapshots: concurrent writes or rotation can change a report.
Queue paths are bounded, but line length, path length, recursion depth, and total
processed bytes are not capped; use trusted, size-controlled logs. Malformed
records are counted but not retained. No private logs are included.

## Discussing this project

[Interview guide](docs/interview-guide.md) explains the threading decisions,
Bash tasks, Linux setup, and a practice checklist. Describe this as a personal
project; associate it with a job or course only if you actually used it there.

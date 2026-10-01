# Interview guide and practice plan

Use the descriptions below only after reading the code, running the project, and
being able to explain the design yourself. This is a new personal project, not
evidence of prior professional or course experience.

## Threading: which API and why?

LogForge uses C++17 `std::thread`, `std::mutex`, and `std::condition_variable`.
It parallelizes independent log files to reduce sequential batch processing time.
A mutex prevents simultaneous queue mutation, condition-variable predicates prevent
incorrect wakeups, and worker-local statistics avoid races in shared counters/maps.
Statistics merge after thread joins. `-pthread` supports the Linux threading runtime;
the implementation does not call the pthreads API directly.

Explain: why a bounded queue provides backpressure; why waiting uses a predicate;
why each worker owns its map; how close unblocks both sides; why errors still join
workers; and why a single input file cannot benefit from this parallelization.

## Bash: what is automated, and where?

In this personal project, Bash scripts automate compilation, tests, synthetic
benchmarking, scheduled report execution, overlap prevention, report publication,
and error-rate checks. `set -euo pipefail` catches command failures and unset
variables. Quoted paths support spaces. `flock` coordinates independent processes;
mutexes coordinate threads inside the C++ process. A temporary file and same-directory
rename prevent readers from observing an incomplete report.

Do not label these scripts as MBS, CMT, or university work unless that is true.

## Linux servers: what runs, and where?

The project was built and tested in a Linux development container. It includes a
runbook and systemd service/timer for a Linux VM, scheduled jobs, service status
checks, and journal monitoring. A remote deployment has not been performed.
After you deploy it yourself, record the VM/provider, dates, commands, and results
before claiming server administration or production support experience.

## Practice tasks

1. Run the example and explain every JSON field.
2. Run 1 and 8 threads; confirm identical counts and compare timings.
3. Set queue capacity to 1; explain why it still finishes.
4. Add a malformed line and explain why it is excluded from valid/error percentage.
5. Use an invalid input path and inspect the nonzero exit status.
6. Run two report wrappers concurrently and explain file locking.
7. Deploy to a Linux VM using the runbook and inspect the systemd journal.
8. Explain the limits of synthetic benchmarks and cache effects.

## Resume wording after hands-on review

"Developed a C++17 log-analysis tool using a bounded producer-consumer queue,
mutexes, condition variables, and worker-local aggregation; validated deterministic
reports across eight concurrency configurations and automated build, tests, and
report monitoring with Bash."

Add Linux deployment claims only after an actual VM deployment. Add performance
figures only with the workload and measured environment, using rerun results you
can explain. Do not turn this new project into a claim about past employment.

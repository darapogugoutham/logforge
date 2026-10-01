# Local validation — October 1, 2026

Environment: Linux x86_64 container, GCC 13.3.0, C++17, GNU Make.

Passed:

- Strict compilation with `-Wall -Wextra -Wpedantic -Werror`.
- Concurrent queue tests: four producers, six consumers, 20,000 items, expected
  count and sum, queue capacity validation, close/drain, blocked-side wakeups.
- Independent report expectations for eight thread/capacity configurations.
- Empty directory, single-file input, malformed records, and seven CLI errors.
- Bash syntax checks for every script; spaced report paths; monitoring exit codes;
  failed runs preserve the previous report; concurrent runs return lock status 75.
- Queue tests under AddressSanitizer and UndefinedBehaviorSanitizer with leak
  detection disabled. LeakSanitizer cannot inspect processes in this container;
  this does not establish leak freedom.
- Queue tests under ThreadSanitizer.
- Synthetic benchmark: 64 files, 640,000 records, three runs at 1/2/4/8 workers;
  exact counts verified in all twelve runs. Raw timings are included separately.

Not completed locally: ShellCheck (not installed), live systemd service deployment,
remote Linux provisioning, and GitHub-hosted CI. Systemd's direct verification
cannot resolve the intended `/opt/logforge` deployment path in this container.
CI is configured to run lint and sanitizer tests after publication; those outcomes
must be checked on GitHub and are not claimed here.

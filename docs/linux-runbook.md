# Linux deployment and operations

Target: a Linux VM with systemd, Bash, GCC, Make, Python 3, and util-linux.
This runbook is provided for deployment; no cloud VM is created by the project.

## Deploy

Build and test the repository before copying it to `/opt/logforge`. On your VM,
from the repository directory:

```bash
bash scripts/build.sh
bash scripts/test.sh
sudo useradd --system --home /var/lib/logforge --shell /usr/sbin/nologin logforge
sudo install -d /opt/logforge /var/lib/logforge /var/log/logforge-input
sudo cp -r build scripts /opt/logforge/
sudo chmod 755 /opt/logforge/scripts/*.sh
sudo chown logforge:logforge /var/lib/logforge
sudo chown root:logforge /var/log/logforge-input
sudo chmod 750 /var/log/logforge-input
sudo install -m 640 -o root -g logforge examples/app.log /var/log/logforge-input/app.log
sudo install -m 644 deploy/logforge.service deploy/logforge.timer /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl start logforge.service
sudo systemctl enable --now logforge.timer
```

If the service user already exists, skip user creation. Keep binaries/scripts
root-owned. Grant the service read access only to selected application logs.
The example timer reanalyzes the complete snapshot every five minutes; reports
are not incremental. Schedule monitoring separately or run it manually:

```bash
bash /opt/logforge/scripts/monitor.sh /var/lib/logforge/latest.json 5
```

The monitor exits 1 when ERROR records exceed the threshold as a percentage of
valid records, 2 when the report/threshold cannot be read, and 0 otherwise.
It does not send alerts. Zero valid records yields 0% errors; inspect malformed
counts to detect format problems.

## Inspect and troubleshoot

```bash
systemctl list-timers logforge.timer
systemctl status logforge.service
journalctl -u logforge.service --since '1 hour ago'
cat /var/lib/logforge/latest.json
```

Failures do not replace the last successful report. Check the journal for input
permissions, missing files, or timeout errors. A oneshot service normally becomes
inactive after success; inspect its exit status rather than expecting a daemon.
The timer does not start a second copy of an already active service. The Bash
wrapper additionally uses a nonblocking file lock for manual concurrent runs.

## Update or stop

Stop the timer before replacing binaries, wait for an active run to finish, test
the new build, copy it in, manually run once, inspect the report, then restart the
timer. Keep the previous binary for rollback.

```bash
sudo systemctl stop logforge.timer
sudo systemctl start logforge.service
sudo systemctl start logforge.timer
# To disable scheduled operation:
sudo systemctl disable --now logforge.timer
```

No scheduled task is installed automatically by build/test scripts. The service
uses a restricted filesystem view, no new privileges, a private temporary directory,
and a five-minute timeout. Report writes are allowed only under `/var/lib/logforge`.

---
name: verify-esp-idf-on-device
description: Build an ESP-IDF firmware change, flash it to the attached ESP32, and watch the serial output until a known success-log line appears (or time out). Use this skill whenever the user asks to verify, test, sanity-check, or "make sure it boots" a code change on real hardware — including after fixing a bug, applying a refactor, before committing, or any time a series of changes wants a per-step smoke test. Also use it whenever they ask to flash, monitor, or run the firmware on the device. Replaces the `idf.py monitor` command which cannot run non-interactively in this harness because it requires a TTY on stdin.
---

# Verify ESP-IDF Firmware on Attached Hardware

A code change on a microcontroller is not verified until the binary has flashed and the device has reached a known-good state. This skill drives that loop end-to-end and is the only way to do it non-interactively in this harness.

## When to invoke

After any code edit that should run on the device — bug fix, refactor, new feature — and before commit. Also when the user says "flash it", "test on hardware", "see if it boots", "monitor the output", or anything similar. If you're applying a series of fixes one-per-commit, run this skill between each fix so a broken intermediate commit gets caught immediately.

## Why this skill exists (and why `idf.py monitor` is not enough)

`idf.py monitor` is interactive — it requires a TTY on stdin and exits with `Error: Monitor requires standard input to be attached to TTY` when invoked non-interactively. Wrapping it in `timeout` or piping it through `tee` does not help; the TTY check happens before any output is produced. This skill bypasses it by talking to the serial device directly via pyserial.

Three other gotchas this skill encodes so you don't re-discover them:

1. **Never use `docker exec`** to reach the container — it runs as root and leaves root-owned build artifacts in your worktree, which then break later container builds. Always use `devcontainer exec --workspace-folder .`. This rule is project policy, not a preference.
2. **Plain `bash -c` skips `export.sh`** — IDF env vars are only set up by `~/.bashrc` under a login shell. Use `bash -lc` (lowercase L) and source `/opt/esp/idf/export.sh` explicitly so `idf.py` resolves.
3. **The S3 needs a DTR/RTS toggle** to leave the boot ROM after flashing. pyserial can do this; resetting the chip via DTR-low + RTS-pulse before reading replicates what `idf.py monitor` does on connect.

## The serial port

This project uses `/dev/ttyACM0` (USB-CDC, ESP32-S3 dev board). Confirm with:

```sh
devcontainer exec --workspace-folder . bash -lc 'ls /dev/ttyACM* /dev/ttyUSB* 2>/dev/null'
```

If the port differs, substitute it throughout. If nothing is returned, the board isn't plugged in or USB passthrough into the container is broken — stop and tell the user; don't fake the verification.

## The full command

This is the canonical invocation. Run it exactly. The success token defaults to `Advertised!` (the log line BLE peripherals emit after `ble_gap_adv_start` succeeds), which is the right signal for this project. Pass a different one if the change you're verifying surfaces somewhere else in boot (e.g. `Stack Syncronized` for a pre-advertisement BLE-stack change).

```sh
devcontainer exec --workspace-folder . bash -lc '
source /opt/esp/idf/export.sh >/dev/null
cd /workspaces/RemoteUnlockArduino
idf.py build 2>&1 | tail -3
idf.py -p /dev/ttyACM0 flash 2>&1 | tail -3
SUCCESS_TOKEN="${SUCCESS_TOKEN:-Advertised!}"
TIMEOUT_S="${TIMEOUT_S:-20}"
python <<PYEOF
import serial, time, sys, os
ser = serial.Serial("/dev/ttyACM0", 115200, timeout=1)
# DTR low + RTS pulse = hardware reset out of bootloader
ser.setDTR(False); ser.setRTS(True); time.sleep(0.1)
ser.setRTS(False); time.sleep(0.1)
token = os.environ["SUCCESS_TOKEN"].encode()
deadline = time.time() + int(os.environ["TIMEOUT_S"])
buf = b""
while time.time() < deadline:
    chunk = ser.read(512)
    if not chunk: continue
    sys.stdout.buffer.write(chunk); sys.stdout.flush()
    buf += chunk
    if token in buf: break
ser.close()
print("\n=== SUCCESS ===" if token in buf else "\n=== TIMEOUT ===")
sys.exit(0 if token in buf else 2)
PYEOF
' 2>&1 | tail -50
```

Three things to know about this command:

- The `tail -50` at the end limits the noise from the IDF env-activation header that fires on every `devcontainer exec`. If the verification times out you need to widen this to see the boot log.
- `SUCCESS_TOKEN` and `TIMEOUT_S` are overridable. Default 20s is enough for a normal cold boot to `Advertised!` (typically ~1.5s). Bump it for changes that delay boot (e.g. an Init retry loop running its full timeout budget).
- The pyserial chunk loop **streams output as it arrives** instead of buffering until exit. This is important — when a change crashes the device in boot, you need the panic backtrace, not "TIMEOUT" with no context.

## How to read the result

- `=== SUCCESS ===` and exit 0 — change boots, BLE stack syncs, advertisement starts. Safe to commit.
- `=== TIMEOUT ===` and exit 2 — device hung, panicked, rebooted into a loop, or your `SUCCESS_TOKEN` is wrong. Read the streamed output above the marker:
  - "Guru Meditation" / backtrace → hardware fault, your change has a real crash bug
  - Continuous reboot loop (the `rst:0x...` line repeats) → likely an `abort()` from inside FreeRTOS task creation or NimBLE init
  - Boot log stops mid-way → a blocking call hangs (typical: `nimble_port_run` entered before init)
  - Boot log looks normal but never reaches your token → your token is wrong for this code path; pick an earlier one and retry
- `Error: Monitor requires standard input to be attached to TTY` → you accidentally invoked `idf.py monitor` instead of this skill. Use the pyserial block above.
- `ModuleNotFoundError: No module named 'serial'` → you forgot to source `export.sh`; pyserial lives in the IDF venv, not the system Python.

## What to do when verification fails

1. **Don't commit.** A failed verify means the change is broken in a way unit/type checking can't catch.
2. **Read the streamed output.** Don't re-run "to see if it was flaky" — embedded boots are deterministic for this kind of failure.
3. **If you can't find the cause from the log, increase verbosity.** `LOG(VERBOSE)` lines around the suspect area in your change, build+flash+monitor again.
4. **If the device is bricked into a USB-detach loop**, you'll need a manual BOOT-button hold while flashing. Tell the user, don't try to force it.

## Project context

This skill targets the firmware at the repo root. The relevant files for verification are:

- `main/main.cpp` — `app_main`, the entry point and the source of the boot-sequence log lines
- `main/abstractions/ble/Advertisement.cpp` — emits `Advertised!` on success
- `sdkconfig` — `CONFIG_BT_NIMBLE_LOG_LEVEL_*` controls how chatty NimBLE is in the monitor output; if you're trying to read the boot log and NimBLE is drowning it, that's the knob

The full devcontainer/build setup is documented in `CLAUDE.md` in the repo root — read it if you're unsure about the environment.

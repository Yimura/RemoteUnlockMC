# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

ESP-IDF firmware for a Bluetooth-LE remote door unlock module. Target: **ESP32-S3** (Xtensa). Framework: **ESP-IDF v6.0** with the NimBLE host stack. Language: C++ (uses C++20 features — `consteval`, concepts, NTTP class templates).

There is no Arduino code despite the repo name; the codebase is pure ESP-IDF.

## Build / flash / monitor

**Always run `idf.py` inside the devcontainer** (`.devcontainer/Dockerfile` pins `espressif/idf:release-v6.0`). Never use `docker exec` from the host — only `devcontainer exec`. The host has no IDF toolchain and `docker exec` runs as root, leaving root-owned build artifacts that break later container builds.

```sh
devcontainer exec --workspace-folder . bash -lc \
  'source /opt/esp/idf/export.sh >/dev/null && idf.py build'
```

`export.sh` is sourced from the container's `.bashrc`, but only under a **login** shell — `bash -lc` works, plain `bash -c` does not.

Common targets (run inside the wrapper above):

```sh
idf.py build
idf.py -p /dev/ttyACM0 flash monitor   # USB-CDC on the S3 dev board → ttyACM0, not ttyUSB0
idf.py menuconfig                      # edit sdkconfig
idf.py reconfigure                     # re-run cmake without rebuilding
idf.py update-dependencies             # bump managed_components per ^semver in idf_component.yml
idf.py fullclean                       # nukes build/ and managed_components/
```

There is **no test suite** and **no lint target**. Code style is enforced by `.clang-format` (Microsoft base, custom alignment rules).

### Verifying a change on hardware

After any code change that's expected to run on the device, build + flash + watch boot. `idf.py monitor` refuses to run without a TTY on stdin (`Error: Monitor requires standard input to be attached to TTY`), so non-interactive verification goes through pyserial directly. The `verify-esp-idf-on-device` skill in `.claude/skills/` wraps the whole loop; the kernel of it is:

```sh
python <<'PYEOF'
import serial, time, sys
ser = serial.Serial("/dev/ttyACM0", 115200, timeout=1)
# DTR low + RTS pulse leaves the chip out of the bootloader after flashing
ser.setDTR(False); ser.setRTS(True); time.sleep(0.1); ser.setRTS(False); time.sleep(0.1)
deadline = time.time() + 20
buf = b""
while time.time() < deadline:
    chunk = ser.read(512)
    if not chunk: continue
    sys.stdout.buffer.write(chunk); sys.stdout.flush()
    buf += chunk
    if b"Advertised!" in buf: break
ser.close()
sys.exit(0 if b"Advertised!" in buf else 2)
PYEOF
```

`pyserial` lives in the IDF venv — source `export.sh` before invoking. `Advertised!` is the log line that fires after `ble_gap_adv_start` succeeds in `Advertisement.cpp`; reaching it means the full boot sequence (NVS, BLE host, GAP/GATT registration, scheduler start, advertisement) worked. Use a different token for changes that surface earlier in boot.

### sdkconfig hygiene across IDF upgrades

The repo ships a committed `sdkconfig`. Casually deleting it drops required options (NimBLE, NVS, partition layout) — see the README's "Checklist for broken project". **However**, on a major-IDF bump (e.g. 5.x → 6.0) sdkconfig contains stale symbols for choices/options that no longer exist; `idf.py reconfigure` migrates the rest of the file once the offending lines are removed. Don't restore the old `sdkconfig` blindly after an IDF upgrade.

Known kconfgen failure mode: if a choice group has all its options set to `n` with no `y` (because the previously-selected option was removed in the new IDF), kconfgen crashes inside its own error-message formatter (`AttributeError: 'NoneType' object has no attribute 'name'`) without identifying the choice. A short Python script that imports `esp_kconfiglib` and walks `kconf.syms` to flag any `sym.choice` with no user-set `y` finds the offending choice in seconds. The 6.0 upgrade tripped this on `APPTRACE_DESTINATION` (commit `2ba41b5`).

Managed components (`espressif/*` via the component manager) often need a matching bump on IDF upgrades — IDF 6.0 trimmed transitive includes, so `idf.py update-dependencies` is part of the upgrade dance, not optional (commit `27e55f1`).

## Architecture

### Globals + namespace

Everything lives in `namespace RemoteUnlock`. Three process-wide singletons are declared `inline` in their headers and referenced directly from `app_main`:

- `g_BleServer` (`abstractions/ble/Ble.hpp`) — NimBLE host wrapper, GAP/GATT init, advertisement, service registry.
- `g_Storage` (`abstractions/storage/Storage.hpp`) — thin template wrapper around NVS (`nvs_get_blob`/`nvs_set_blob`) under namespace `"storage"`.
- `g_Scheduler` (`abstractions/scheduler/Scheduler.hpp`) — periodic-job runner on a dedicated FreeRTOS task.

Boot sequence (`main/main.cpp`):
1. `LogHelper` ctor → `Logger::Init()` + console sink (AsyncLogger).
2. `g_Storage.Init()` — NVS flash init.
3. Construct services (`DoorService`, `StatusService`, `SettingsService`) — their ctors register characteristics + service with `g_BleServer`.
4. `g_BleServer.Init()` — GAP, GATT, advertisement.
5. `g_Scheduler.Start(SCHEDULER_UPDATE_RATE)` — spawns the scheduler task.
6. `g_BleServer.Run()` — **blocking**; nothing below it executes.

### BLE service pattern

`abstractions/ble/` provides `BleService` + `BleCharacteristic`. A feature service (e.g. `DoorService`) is a class that owns:
- one `BleService` member built from a UUID in `services/ServiceDefinitions.hpp`,
- one `BleCharacteristic` member per characteristic, each constructed with a lambda dispatching to a `*ChrAccess` / `*ChrWrite` member function,
- any peripherals (e.g. `GPIO<>`, `OnboardLed<>`).

The service ctor calls `m_BleService.RegisterCharacteristic(...)` for each characteristic then `g_BleServer.RegisterService(m_BleService)`. UUIDs for the three services and their characteristics are declared as `constexpr ble_uuid128_t` literals in the service headers.

To **add a new BLE service**: define UUIDs in `ServiceDefinitions.hpp`, create a `<Name>Service.{hpp,cpp}` in `main/services/` following the existing pattern, then construct an instance in `app_main` **before** `g_BleServer.Init()`.

### NimBLE patterns to follow when extending the BLE layer

These are encoded in the abstractions but easy to misuse when writing the next service:

- **Register characteristics through `BleService::Register(chr1, chr2, ...)`**, not via separate `RegisterCharacteristic` + `g_BleServer.RegisterService` calls. The variadic `Register` couples the two so a new characteristic can't be silently absent from the GATT table because someone forgot the second line.
- **The access callback dispatcher uses NimBLE's per-characteristic `arg` pointer** (`chr_def.arg = this` set in `BleCharacteristic::Build`). Don't reintroduce a separate registry or attr-handle lookup.
- **Reads from `os_mbuf` must use `MbufReadExact` / `MbufReadPartial` / `MbufReadString` from `Helpers.hpp`**, which walk the full chained payload via `OS_MBUF_PKTLEN` + `ble_hs_mbuf_to_flat`. Inspecting `om_len` directly only sees the head segment and silently truncates long writes split across mbufs.
- **Cross-task state needs `std::atomic`.** Characteristic values written by the `g_Scheduler` task and read by the NimBLE host task (e.g. `StatusService::m_Voltage`) are a data race under the C++ memory model even when aligned 32-bit access happens to be atomic on Xtensa LX7. Use `std::atomic<T>` with `memory_order_relaxed`.
- **Access callbacks should return `BLE_ATT_ERR_*` on bad input**, not 0. A peer writing a payload that fails `MbufReadPartial`'s size check should see the error so it doesn't think the write succeeded (see `DoorService::DoorLockToggleChrWrite`).

### Template abstractions

Several abstractions are header-only templates parameterized at compile time:

- `GPIO<PinNumber, PinMode>` — pin number + mode are template parameters; `Read()`/`Toggle()` are `requires`-gated to the matching mode.
- `OnboardLed<PinNumber>` — wraps the `espressif/led_strip` managed component (single WS2812 on the dev board, GPIO 48). Predefined `WHITE/RED/GREEN/BLUE` colors.
- `StorageItem<Id, T>` — NVS-backed value with a compile-time string key (`StorageItemId` NTTP) and a lazy-read cache. Used by `Ble` for the persisted device name (`BLE_DEV_NAME`).

When you see e.g. `StorageItem<"BLE_DEV_NAME", char[20]>`, the string literal is a non-type template parameter — changing the key changes the type.

### Logging

`LOG(LEVEL) << ...` macros come from the external **AsyncLogger** library (`Yimura/AsyncLogger`, fetched via `cmake/AsyncLogger.cmake` using `FetchContent`). `LogHelper` installs a console sink that prints `[LEVEL/file:line] msg`. The logger is `using namespace al;` in `common.hpp`, which is the **precompiled header** for the `main` component — symbols from `al::` are visible everywhere without including `Logger.hpp`.

### Scheduler

`g_Scheduler.AddJob(callback, interval_ms)` queues a periodic callback. `Start(update_rate_ms)` spawns a FreeRTOS task that loops, ticks `vTaskDelay(update_rate_ms)`, and fires every job whose `m_next_fire_time_ms` has elapsed. Jobs run on the scheduler task, not the BLE host task — keep callbacks short and avoid blocking BLE APIs from them.

## Component / dependency layout

- `main/CMakeLists.txt` uses `file(GLOB_RECURSE)` on `*.cpp`/`*.hpp` under `main/`, so new files in `main/` or its subdirs are picked up without editing CMake. Required IDF components: `bt`, `esp_driver_gpio`, `nvs_flash`, `led_strip`.
- `main/common.hpp` is set as the precompiled header — keep it small; anything added here recompiles every TU.
- Managed components (`managed_components/`) are fetched by ESP-IDF component manager from `main/idf_component.yml` + `dependencies.lock`. Only `espressif/led_strip ^3.0.1` is declared; the lock currently pins `3.0.3` (3.0.1/3.0.2 don't build against IDF 6.0).
- AsyncLogger is **not** an IDF component; it's pulled via plain CMake `FetchContent` and linked into `${COMPONENT_LIB}` from `main/CMakeLists.txt`.

## Conventions worth knowing

- Member fields use `m_PascalCase`; constants `UPPER_SNAKE`; globals `g_PascalCase`.
- Services and abstractions own their members by value; copy/move are explicitly deleted on `Ble`. Don't introduce dynamic allocation where the existing pattern uses members.
- `ble_uuid128_t` literals are written **byte-reversed** from their string form (see `ServiceDefinitions.hpp`) — match the existing pattern when adding UUIDs.
- Pin assignments live at the point of use (e.g. `GPIO<GPIO_NUM_33, GPIO_MODE_OUTPUT> m_DoorRelay` in `DoorService.hpp`), not in a central pin map.

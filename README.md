# Remote Unlock

Remote Unlock code for ESP-32

It contains several C++ template abstractions simplifying the use of ESP-32 library functions, most of these are either work in progress and/or untested.

## Abstractions

- BLE : Not really an abstraction at this point (2025-01-23)
- GPIO : Self explanatory
- Storage (and StorageItem) : Abstraction around NVS

## Checklist for broken project

- Are the required components defined in main/CMakeLists.txt
- Did you create a new sdkconfig? This might have disabled certain features required for this project (nimble_ble, nvs_flash, ...)

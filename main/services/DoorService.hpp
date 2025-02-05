#pragma once
#include <array>
#include <span>

#include "abstractions/ble/Characteristic.hpp"
#include "abstractions/ble/Service.hpp"

namespace RemoteUnlock
{
    class DoorService final
    {
    private:
        BleService m_BleService = BleService({.u16 = BLE_UUID16_INIT(0x1337)}, BLE_GATT_SVC_TYPE_PRIMARY);

        BleCharacteristic m_DoorLockStateCharacteristic =
            BleCharacteristic({.u16 = BLE_UUID16_INIT(0x1338)}, BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_INDICATE,
                [this](uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg) -> int {
            return DoorLockStateChrAccess(conn_handle, attr_handle, ctxt, arg);
        });

        BleCharacteristic m_DoorLockToggleCharacteristic =
            BleCharacteristic({.u16 = BLE_UUID16_INIT(0x1339)}, BLE_GATT_CHR_F_WRITE,
                [this](uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg) -> int {
            return DoorLockToggleChrWrite(conn_handle, attr_handle, ctxt, arg);
        });

    public:
        DoorService();
        virtual ~DoorService();

        int DoorLockStateChrAccess(uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg);
        int DoorLockToggleChrWrite(uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg);
    };
} // namespace RemoteUnlock

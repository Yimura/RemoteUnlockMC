#pragma once
#include "abstractions/ble/Characteristic.hpp"
#include "abstractions/ble/Service.hpp"
#include "abstractions/gpio/GPIO.hpp"
#include "ServiceDefinitions.hpp"

namespace RemoteUnlock
{
    // f2eb002a-7e74-4cc5-ac27-9c66f4c0c6b2
    constexpr ble_uuid128_t DoorLockStateChrUUID = BLE_UUID128_INIT(
        0xb2, 0xc6, 0xc0, 0xf4, 0x66, 0x9c, 0x27, 0xac, 0xc5, 0x4c, 0x74, 0x7e, 0x2a, 0x00, 0xeb, 0xf2);
    // 78943d9a-fb5b-4580-9ec8-131d092b4b78
    constexpr ble_uuid128_t DoorLockWriteChrUUID = BLE_UUID128_INIT(
        0x78, 0x4b, 0x2b, 0x09, 0x1d, 0x13, 0xc8, 0x9e, 0x80, 0x45, 0x5b, 0xfb, 0x9a, 0x3d, 0x94, 0x78);

    class DoorService final
    {
    private:
        BleService m_BleService = BleService({.u128 = DoorServiceUUID}, BLE_GATT_SVC_TYPE_PRIMARY);

        BleCharacteristic m_DoorLockStateCharacteristic =
            BleCharacteristic({.u128 = DoorLockStateChrUUID}, BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_INDICATE,
                [this](uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg) -> int {
            return DoorLockStateChrAccess(conn_handle, attr_handle, ctxt, arg);
        });

        BleCharacteristic m_DoorLockToggleCharacteristic =
            BleCharacteristic({.u128 = DoorLockWriteChrUUID}, BLE_GATT_CHR_F_WRITE,
                [this](uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg) -> int {
            return DoorLockToggleChrWrite(conn_handle, attr_handle, ctxt, arg);
        });

        GPIO<GPIO_NUM_33, GPIO_MODE_OUTPUT> m_DoorRelay{};
        bool m_DoorState;

    public:
        DoorService();
        virtual ~DoorService() = default;

        int DoorLockStateChrAccess(uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg);
        int DoorLockToggleChrWrite(uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg);
    };
} // namespace RemoteUnlock

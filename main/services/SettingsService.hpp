#pragma once
#include "abstractions/ble/Characteristic.hpp"
#include "abstractions/ble/Service.hpp"
#include "ServiceDefinitions.hpp"

namespace RemoteUnlock
{
    // 0ab8e741-ede3-4a49-9d80-55f96354bc7a
    constexpr ble_uuid128_t SetNameChrUUID = BLE_UUID128_INIT(
        0x7a, 0xbc, 0x54, 0x63, 0xf9, 0x55, 0x80, 0x9d, 0x49, 0x4a, 0xe3, 0xed, 0x41, 0xe7, 0xb8, 0x0a);

    class SettingsService
    {
    private:
        BleService m_BleService = BleService({.u128 = SettingsServiceUUID}, BLE_GATT_SVC_TYPE_PRIMARY);

        BleCharacteristic m_SetDeviceNameCharacteristic =
            BleCharacteristic({.u128 = SetNameChrUUID}, BLE_GATT_CHR_F_WRITE,
                [this](uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg) -> int {
            return DeviceNameModifyChrWrite(conn_handle, attr_handle, ctxt, arg);
        });

    public:
        SettingsService();
        virtual ~SettingsService() = default;

        int DeviceNameModifyChrWrite(uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg);
    };
} // namespace RemoteUnlock

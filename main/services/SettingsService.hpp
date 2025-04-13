#pragma once
#include "abstractions/ble/Characteristic.hpp"
#include "abstractions/ble/Service.hpp"

namespace RemoteUnlock
{
    class SettingsService
    {
    private:
        BleService m_BleService = BleService({.u16 = BLE_UUID16_INIT(0x9595)}, BLE_GATT_SVC_TYPE_PRIMARY);

        BleCharacteristic m_DeviceNameModifyCharacteristic =
            BleCharacteristic({.u16 = BLE_UUID16_INIT(0x9596)}, BLE_GATT_CHR_F_WRITE,
                [this](uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg) -> int {
            return DeviceNameModifyChrWrite(conn_handle, attr_handle, ctxt, arg);
        });

    public:
        SettingsService();
        virtual ~SettingsService() = default;

        int DeviceNameModifyChrWrite(uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg);
    };
} // namespace RemoteUnlock

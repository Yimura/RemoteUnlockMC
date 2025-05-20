#pragma once
#include "abstractions/ble/Characteristic.hpp"
#include "abstractions/ble/Service.hpp"
#include "ServiceDefinitions.hpp"

namespace RemoteUnlock
{
    // 2a6c7a4f-22bb-45da-8eed-3839e6826adf
    constexpr ble_uuid128_t VoltageChrAccessUUID = BLE_UUID128_INIT(
        0xdf, 0x6a, 0x82, 0xe6, 0x39, 0x38, 0xed, 0x8e, 0xda, 0x45, 0xbb, 0x22, 0x4f, 0x7a, 0x6c, 0x2a);

    class StatusService
    {
    private:
        BleService m_BleService = BleService({.u128 = StatusServiceUUID}, BLE_GATT_SVC_TYPE_PRIMARY);

        BleCharacteristic m_VoltageChrAccess =
            BleCharacteristic({.u128 = VoltageChrAccessUUID}, BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_INDICATE,
                [this](uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg) -> int {
            return VoltageChrAccess(conn_handle, attr_handle, ctxt, arg);
        });

        float m_Voltage = 14.5;

    public:
        StatusService();
        virtual ~StatusService() = default;

        int VoltageChrAccess(uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg);
    };
} // namespace RemoteUnlock

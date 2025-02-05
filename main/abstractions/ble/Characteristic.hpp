#pragma once
#include <cstdint>
#include <functional>
#include <vector>

#include <host/ble_gatt.h>
#include <host/ble_uuid.h>

namespace RemoteUnlock
{
    using BleChrAccessCb =
        std::function<int(uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg)>;

    class BleCharacteristic
    {
    private:
        ble_uuid_any_t m_Uuid;
        uint16_t m_Flags;
        BleChrAccessCb m_Callback;
        uint16_t m_ValueHandle;

    public:
        BleCharacteristic(ble_uuid_any_t uuid, uint16_t flags, BleChrAccessCb access_callback);
        virtual ~BleCharacteristic();

        ble_gatt_chr_def Build();

    private:
        inline static std::vector<BleCharacteristic*> m_Characteristics = {};
        static int CharacteristicAccessCallback(
            uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg);
    };
} // namespace RemoteUnlock

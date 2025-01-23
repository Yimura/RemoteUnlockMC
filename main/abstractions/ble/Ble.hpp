#pragma once
#include "abstractions/storage/StorageItem.hpp"

struct ble_gatt_register_ctxt;

namespace RemoteUnlock
{
    class Ble
    {
    private:
        StorageItem<"BLE_DEV_NAME", char[20]> m_DeviceName = StorageItem<"BLE_DEV_NAME", char[20]>("BMW E36");

    public:
        Ble()                          = default;
        virtual ~Ble()                 = default;
        Ble(const Ble&)                = default;
        Ble(Ble&&) noexcept            = default;
        Ble& operator=(const Ble&)     = default;
        Ble& operator=(Ble&&) noexcept = default;

        void GattSvrRegisterCb(struct ble_gatt_register_ctxt* ctxt, void* arg);
        void OnStackSync();
        void OnStackErr(int reason);

        void Destroy();
        bool Init();

        void NimbleHostConfigInit();
    };

    inline Ble g_BleServer{};
} // namespace RemoteUnlock

#pragma once

struct ble_gatt_register_ctxt;

namespace RemoteUnlock
{
    class Ble
    {
    private:
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

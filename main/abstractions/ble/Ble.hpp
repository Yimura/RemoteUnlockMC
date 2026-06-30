#pragma once

#include <unordered_set>
#include <vector>

#include <host/ble_hs.h>
#include <host/ble_uuid.h>
#include <host/util/util.h>
#include <nimble/ble.h>
#include <nimble/nimble_port.h>
#include <nimble/nimble_port_freertos.h>
#include <services/gatt/ble_svc_gatt.h>
#include <services/gap/ble_svc_gap.h>
#include <store/config/ble_store_config.h>

#include "Service.hpp"

#include "abstractions/storage/StorageItem.hpp"

namespace RemoteUnlock
{
    // Fixed passkey injected on BLE_GAP_EVENT_PASSKEY_ACTION. Central must enter
    // this value to complete LE Secure Connections pairing with MITM protection.
    // Provisional shared secret — revisit once a per-device secret or OOB flow lands.
    constexpr uint32_t BLE_FIXED_PASSKEY = 123456;

    class Ble
    {
    private:
        StorageItem<"BLE_DEV_NAME", char[20]> m_DeviceName = StorageItem<"BLE_DEV_NAME", char[20]>(DEFAULT_DEVICE_NAME);

        std::vector<ble_gatt_svc_def> m_Services;

        bool m_PortInitialized = false;

    public:
        Ble();
        virtual ~Ble()                 = default;
        Ble(const Ble&)                = delete;
        Ble(Ble&&) noexcept            = delete;
        Ble& operator=(const Ble&)     = delete;
        Ble& operator=(Ble&&) noexcept = delete;

        void Destroy();
        bool GapInit();
        bool GattInit();
        bool Init();
        void Run();

        const char* GetDeviceName();
        bool SetDeviceName(const char* new_name);

        // Advertisement.cpp
        bool AdvertisementInit();
        bool AdvertisementStart();

        // Gapp.cpp
        int GapEventHandler(ble_gap_event* event, void* args);
        int GapEventConnect(ble_gap_event* event);
        int GapEventDisconnect(ble_gap_event* event);
        int GapEventConnUpdate(ble_gap_event* event);
        int GapEventAdvertisementComplete(ble_gap_event* event);
        int GapEventSubscribe(ble_gap_event* event);
        int GapEventMtuUpdate(ble_gap_event* event);
        int GapEventPasskeyAction(ble_gap_event* event);
        int GapEventEncryptionChange(ble_gap_event* event);
        int GapEventRepeatPairing(ble_gap_event* event);
        int GapEventIdentityResolved(ble_gap_event* event);

        // Conn handles that observed BLE_GAP_EVENT_PASSKEY_ACTION since their
        // most recent BLE_GAP_EVENT_CONNECT. Used by GapEventEncryptionChange
        // to label the encryption as FRESH-PAIRING vs REUSED-BOND.
        std::unordered_set<uint16_t> m_PairingConnHandles;

        // Gatt.cpp
        void GattSvrSubscribe(ble_gap_event* event);
        void GattSvrRegisterCb(struct ble_gatt_register_ctxt* ctxt, void* arg);

        void OnStackSync();
        void OnStackErr(int reason);

        void RegisterService(BleService& service);

    private:
        void NimbleHostConfigInit();
    };

    inline Ble g_BleServer{};
} // namespace RemoteUnlock

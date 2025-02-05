#pragma once

#include <functional>
#include <unordered_map>
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
    using GapEventCb = int (*)(ble_gap_event*);

    class Ble
    {
    private:
        StorageItem<"BLE_DEV_NAME", char[20]> m_DeviceName = StorageItem<"BLE_DEV_NAME", char[20]>("BMW E36");

        std::unordered_map<int, GapEventCb> m_GapEventHandlers;
        std::vector<ble_gatt_svc_def> m_Services;

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

        // Gatt.cpp
        void GattSvrSubscribe(ble_gap_event* event);
        void GattSvrRegisterCb(struct ble_gatt_register_ctxt* ctxt, void* arg);

        void OnStackSync();
        void OnStackErr(int reason);

        void RegisterService(BleService& service);

    private:
        void RegisterEventHandlers();

        void NimbleHostConfigInit();
    };

    inline Ble g_BleServer{};
} // namespace RemoteUnlock

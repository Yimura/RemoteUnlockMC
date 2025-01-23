#include "Ble.hpp"
#include <host/ble_hs.h>
#include <host/ble_uuid.h>
#include <host/util/util.h>
#include <nimble/ble.h>
#include <nimble/nimble_port.h>
#include <nimble/nimble_port_freertos.h>
#include <services/gatt/ble_svc_gatt.h>
#include <services/gap/ble_svc_gap.h>
#include <functional>
#include <store/config/ble_store_config.h>

extern "C" void ble_store_config_init(void);

namespace RemoteUnlock
{
    constexpr char DEVICE_NAME[] = "BMW E36";

    static int door_lock_chr_read(uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg);

    const ble_uuid16_t svc_uuid             = BLE_UUID16_INIT(0x1337);
    const ble_uuid16_t door_lock_state_uuid = BLE_UUID16_INIT(0x1338);
    uint16_t door_lock_state_handle;

    constexpr ble_gatt_chr_def door_lock_state_chr = {
        .uuid       = &door_lock_state_uuid.u,
        .access_cb  = &door_lock_chr_read,
        .flags      = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_INDICATE,
        .val_handle = &door_lock_state_handle,
    };

    constexpr ble_gatt_svc_def gatt_svr_svcs[] = {
        {
         .type            = BLE_GATT_SVC_TYPE_PRIMARY,
         .uuid            = &svc_uuid.u,
         .characteristics = {&door_lock_state_chr},
         },
    };

    void Ble::GattSvrRegisterCb(ble_gatt_register_ctxt* ctxt, void* arg)
    {
    }

    void Ble::OnStackSync()
    {
    }

    void Ble::Destroy()
    {
        nimble_port_stop();
    }

    bool Ble::Init()
    {
        esp_err_t result = nimble_port_init();
        if (result != ESP_OK)
        {
            return false;
        }


#pragma region GAP
        ble_svc_gap_init();
        if (ble_svc_gap_device_name_set(DEVICE_NAME) != 0)
        {
            return false;
        }
#pragma endregion

        ble_svc_gatt_init();
        if (ble_gatts_count_cfg(gatt_svr_svcs) != 0)
        {
            return false;
        }

        if (ble_gatts_add_svcs(gatt_svr_svcs) != 0)
        {
            return false;
        }

        NimbleHostConfigInit();
        return true;
    }

    void Ble::NimbleHostConfigInit()
    {
        ble_hs_cfg.gatts_register_cb = [](ble_gatt_register_ctxt* ctxt, void* arg) {
            g_BleServer.GattSvrRegisterCb(ctxt, arg);
        };
        ble_hs_cfg.sync_cb         = []() { g_BleServer.OnStackSync(); };
        ble_hs_cfg.reset_cb        = [](int reason) { g_BleServer.OnStackErr(reason); };
        ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

        ble_store_config_init();
    }
} // namespace RemoteUnlock

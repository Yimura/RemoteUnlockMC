#include "Ble.hpp"

#include <functional>
#include <host/ble_hs.h>
#include <host/ble_uuid.h>
#include <host/util/util.h>
#include <nimble/ble.h>
#include <nimble/nimble_port.h>
#include <nimble/nimble_port_freertos.h>
#include <services/gatt/ble_svc_gatt.h>
#include <services/gap/ble_svc_gap.h>
#include <store/config/ble_store_config.h>

#include "Helper.hpp"

extern "C" void ble_store_config_init(void);

namespace RemoteUnlock
{
    static int door_lock_chr_read(uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg)
    {
        return 0;
    }

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
         .characteristics = (struct ble_gatt_chr_def[]){door_lock_state_chr, EMPTY_GATT_CHR_DEF()},
         },
        EMPTY_GATT_SVC_DEF()
    };

    void Ble::GattSvrRegisterCb(ble_gatt_register_ctxt* ctxt, void* arg)
    {
        char bleUUID[BLE_UUID_STR_LEN];

        switch (ctxt->op)
        {
        case BLE_GATT_REGISTER_OP_SVC:
            std::cout << "Registered bluetooth service " << ble_uuid_to_str(ctxt->svc.svc_def->uuid, bleUUID)
                      << " with handle=" << ctxt->svc.handle << std::endl;

            break;

        case BLE_GATT_REGISTER_OP_CHR:
            std::cout << "Registered bluetooth characteristic " << ble_uuid_to_str(ctxt->chr.chr_def->uuid, bleUUID)
                      << " with handle=" << ctxt->chr.def_handle << std::endl;

            break;

        case BLE_GATT_REGISTER_OP_DSC:
            std::cout << "Registered bluetooth descriptor " << ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, bleUUID)
                      << " with handle=" << ctxt->dsc.handle << std::endl;

            break;

        default:
            std::cout << "Unknown event caught: " << ctxt->op << std::endl;

            break;
        }
    }

    void Ble::OnStackSync()
    {
        std::cout << "Stack Syncronized." << std::endl;
    }

    void Ble::OnStackErr(int reason)
    {
        std::cout << "Stack error with reason: " << reason << std::endl;
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
        auto deviceName = m_DeviceName.Get();

        ble_svc_gap_init();
        if (ble_svc_gap_device_name_set(deviceName) != 0)
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

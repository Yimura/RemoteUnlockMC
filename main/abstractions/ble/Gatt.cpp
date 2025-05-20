#include "Ble.hpp"

#include "Helpers.hpp"

namespace RemoteUnlock
{
    bool RemoteUnlock::Ble::GattInit()
    {
        m_Services.push_back(EMPTY_GATT_SVC_DEF());

        ble_svc_gatt_init();
        if (ble_gatts_count_cfg(m_Services.data()) != 0)
        {
            LOG(FATAL) << "Failed to count Gatt Services.";

            return false;
        }

        if (ble_gatts_add_svcs(m_Services.data()) != 0)
        {
            LOG(FATAL) << "Failed to add Gatt services.";

            return false;
        }

        return true;
    }

    void Ble::GattSvrRegisterCb(ble_gatt_register_ctxt* ctxt, void* arg)
    {
        char bleUUID[BLE_UUID_STR_LEN];

        switch (ctxt->op)
        {
        case BLE_GATT_REGISTER_OP_SVC:
            LOG(VERBOSE) << "Registered bluetooth service " << ble_uuid_to_str(ctxt->svc.svc_def->uuid, bleUUID)
                         << " with handle=" << ctxt->svc.handle;

            break;

        case BLE_GATT_REGISTER_OP_CHR:
            LOG(VERBOSE) << "Registered bluetooth characteristic " << ble_uuid_to_str(ctxt->chr.chr_def->uuid, bleUUID)
                         << " with handle=" << ctxt->chr.def_handle;

            break;

        case BLE_GATT_REGISTER_OP_DSC:
            LOG(VERBOSE) << "Registered bluetooth descriptor " << ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, bleUUID)
                         << " with handle=" << ctxt->dsc.handle;

            break;

        default:
            LOG(WARNING) << "Unknown event caught: " << ctxt->op;

            break;
        }
    }

    void Ble::GattSvrSubscribe(ble_gap_event* event)
    {
    }
} // namespace RemoteUnlock

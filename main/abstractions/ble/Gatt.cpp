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
            std::cout << "Failed to count Gatt Services." << std::endl;

            return false;
        }

        if (ble_gatts_add_svcs(m_Services.data()) != 0)
        {
            std::cout << "Failed to add Gatt services." << std::endl;

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

    void Ble::GattSvrSubscribe(ble_gap_event* event)
    {
    }
} // namespace RemoteUnlock

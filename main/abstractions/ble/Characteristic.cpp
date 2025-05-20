#include "Characteristic.hpp"

#include <algorithm>

namespace RemoteUnlock
{
    BleCharacteristic::BleCharacteristic(ble_uuid_any_t uuid, uint16_t flags, BleChrAccessCb access_callback)
        : m_Uuid(uuid), m_Flags(flags), m_Callback(access_callback), m_ValueHandle(0)
    {
        BleCharacteristic::m_Characteristics.push_back(this);
    }

    BleCharacteristic::~BleCharacteristic()
    {
        auto _ = std::remove(m_Characteristics.begin(), m_Characteristics.end(), this);
    }

    ble_gatt_chr_def BleCharacteristic::Build()
    {
        return ble_gatt_chr_def{
            .uuid       = &m_Uuid.u,
            .access_cb  = BleCharacteristic::CharacteristicAccessCallback,
            .flags      = m_Flags,
            .val_handle = &m_ValueHandle,
        };
    }

    bool BleCharacteristic::Indicate(uint16_t conn_handle)
    {
        return ble_gatts_indicate(conn_handle, m_ValueHandle);
    }

    int BleCharacteristic::CharacteristicAccessCallback(
        uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg)
    {
        if (conn_handle == (uint16_t)-1)
        {
            LOG(VERBOSE) << "characteristic access by nimble stack";
        }

        for (const auto& characteristic : m_Characteristics)
        {
            // match our specific characteristic and make sure it has a valid callback
            if (attr_handle == characteristic->m_ValueHandle && characteristic->m_Callback)
            {
                bool can_access_attr =
                    ((characteristic->m_Flags & BLE_GATT_CHR_F_WRITE) && ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) ||
                    (characteristic->m_Flags & BLE_GATT_CHR_F_READ && ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR);
                if (can_access_attr)
                {
                    return characteristic->m_Callback(conn_handle, attr_handle, ctxt, arg);
                }
            }
        }
        return 0;
    }
} // namespace RemoteUnlock

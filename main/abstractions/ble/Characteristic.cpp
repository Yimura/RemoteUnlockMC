#include "Characteristic.hpp"

namespace RemoteUnlock
{
    BleCharacteristic::BleCharacteristic(ble_uuid_any_t uuid, uint16_t flags, BleChrAccessCb access_callback)
        : m_Uuid(uuid), m_Flags(flags), m_Callback(access_callback), m_ValueHandle(0)
    {
    }

    ble_gatt_chr_def BleCharacteristic::Build()
    {
        return ble_gatt_chr_def{
            .uuid       = &m_Uuid.u,
            .access_cb  = BleCharacteristic::CharacteristicAccessCallback,
            .arg        = this,
            .flags      = m_Flags,
            .val_handle = &m_ValueHandle,
        };
    }

    bool BleCharacteristic::Indicate(uint16_t conn_handle)
    {
        return ble_gatts_indicate(conn_handle, m_ValueHandle) == 0;
    }

    int BleCharacteristic::CharacteristicAccessCallback(
        uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg)
    {
        if (conn_handle == (uint16_t)-1)
        {
            LOG(VERBOSE) << "characteristic access by nimble stack";
        }

        auto* characteristic = static_cast<BleCharacteristic*>(arg);
        if (!characteristic || !characteristic->m_Callback)
        {
            return 0;
        }

        const bool can_access_attr =
            ((characteristic->m_Flags & BLE_GATT_CHR_F_WRITE) && ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) ||
            ((characteristic->m_Flags & BLE_GATT_CHR_F_READ) && ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR);
        if (!can_access_attr)
        {
            return 0;
        }
        return characteristic->m_Callback(conn_handle, attr_handle, ctxt, arg);
    }
} // namespace RemoteUnlock

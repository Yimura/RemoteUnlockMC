#include "Ble.hpp"

#include "Helper.hpp"

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

    void Ble::GattSvrSubscribe(ble_gap_event* event)
    {
    }
} // namespace RemoteUnlock

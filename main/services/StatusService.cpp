#include "StatusService.hpp"
#include "abstractions/ble/Ble.hpp"
#include "abstractions/scheduler/Scheduler.hpp"
#include "abstractions/random/Random.hpp"

namespace RemoteUnlock
{
    StatusService::StatusService()
    {
        m_BleService.Register(m_VoltageChrAccess);

        g_Scheduler.AddJob([this] { UpdateStatusElements(); }, 5000);
    }

    int StatusService::VoltageChrAccess(
        uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg)
    {
        const float voltage = m_Voltage.load(std::memory_order_relaxed);
        MbufAppend(ctxt->om, voltage);

        return 0;
    }

    void StatusService::UpdateStatusElements()
    {
        m_Voltage.store(Random::BetweenFloat(10.5f, 14.f), std::memory_order_relaxed);
    }
} // namespace RemoteUnlock

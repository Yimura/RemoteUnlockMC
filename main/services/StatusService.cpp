#include "StatusService.hpp"
#include "abstractions/ble/Ble.hpp"
#include "abstractions/scheduler/Scheduler.hpp"
#include "abstractions/random/Random.hpp"

namespace RemoteUnlock
{
    StatusService::StatusService()
    {
        m_BleService.RegisterCharacteristic(m_VoltageChrAccess);
        g_BleServer.RegisterService(m_BleService);

        g_Scheduler.AddJob([this] { UpdateStatusElements(); }, 5000);
    }

    int StatusService::VoltageChrAccess(
        uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg)
    {
        MbufAppend(ctxt->om, m_Voltage);

        return 0;
    }

    void StatusService::UpdateStatusElements()
    {
        m_Voltage = Random::BetweenFloat(10.5f, 14.f);
    }
} // namespace RemoteUnlock

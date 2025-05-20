#include "StatusService.hpp"
#include "abstractions/ble/Ble.hpp"

namespace RemoteUnlock
{
    StatusService::StatusService()
    {
        m_BleService.RegisterCharacteristic(m_VoltageChrAccess);
        g_BleServer.RegisterService(m_BleService);
    }

    int StatusService::VoltageChrAccess(
        uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg)
    {
        MbufAppend(ctxt->om, m_Voltage);

        return 0;
    }
} // namespace RemoteUnlock

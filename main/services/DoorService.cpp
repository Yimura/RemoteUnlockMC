#include "DoorService.hpp"

#include "abstractions/ble/Ble.hpp"

namespace RemoteUnlock
{
    DoorService::DoorService() : m_DoorState(false)
    {
        m_BleService.RegisterCharacteristic(m_DoorLockStateCharacteristic);
        m_BleService.RegisterCharacteristic(m_DoorLockToggleCharacteristic);
        g_BleServer.RegisterService(m_BleService);
    }

    DoorService::~DoorService()
    {
    }

    int DoorService::DoorLockStateChrAccess(
        uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg)
    {
        MbufAppend(ctxt->om, m_DoorState);

        return 0;
    }

    int DoorService::DoorLockToggleChrWrite(
        uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg)
    {
        MbufReadPartial(ctxt->om, m_DoorState);

        return 0;
    }
} // namespace RemoteUnlock

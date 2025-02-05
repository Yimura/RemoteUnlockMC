#include "DoorService.hpp"

#include "abstractions/ble/Ble.hpp"

namespace RemoteUnlock
{
    DoorService::DoorService()
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
        return 0;
    }

    int DoorService::DoorLockToggleChrWrite(
        uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg)
    {
        return 0;
    }
} // namespace RemoteUnlock

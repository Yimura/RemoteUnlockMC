#include "SettingsService.hpp"
#include "abstractions/ble/Ble.hpp"

namespace RemoteUnlock
{
    SettingsService::SettingsService()
    {
        m_BleService.Register(m_SetDeviceNameCharacteristic);
    }

    int SettingsService::DeviceNameModifyChrWrite(
        uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg)
    {
        const auto newName = MbufReadString(ctxt->om);
        g_BleServer.SetDeviceName(newName.c_str());

        return 0;
    }
} // namespace RemoteUnlock

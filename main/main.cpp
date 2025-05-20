#include "common.hpp"
#include "abstractions/gpio/GPIO.hpp"
#include "abstractions/storage/Storage.hpp"
#include "abstractions/storage/StorageItem.hpp"
#include "abstractions/ble/Ble.hpp"

#include "services/DoorService.hpp"
#include "services/StatusService.hpp"
#include "services/SettingsService.hpp"

#include "LogHelper.hpp"

extern "C" void app_main()
{
    using namespace RemoteUnlock;

    auto logger = LogHelper();

    LOG(INFO) << "Initializing Flash Storage";
    g_Storage.Init();

    auto deviceName = g_BleServer.GetDeviceName();
    LOG(INFO) << "Device Name: " << deviceName;

    auto doorService     = DoorService();
    auto statusService   = StatusService();
    auto settingsService = SettingsService();

    g_BleServer.Init();
    g_BleServer.Run();

    g_Storage.Destroy();

    return;
}

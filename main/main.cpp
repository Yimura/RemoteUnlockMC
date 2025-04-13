#include "common.hpp"
#include "abstractions/gpio/GPIO.hpp"
#include "abstractions/storage/Storage.hpp"
#include "abstractions/storage/StorageItem.hpp"
#include "abstractions/ble/Ble.hpp"

#include "services/DoorService.hpp"

extern "C" void app_main()
{
    using namespace RemoteUnlock;

    std::cout << "Initializing Flash Storage" << std::endl;
    g_Storage.Init();

    auto deviceName = g_BleServer.GetDeviceName();
    std::cout << "Device Name: " << deviceName << std::endl;

    auto doorService = DoorService();

    g_BleServer.Init();
    g_BleServer.Run();

    g_Storage.Destroy();

    return;
}

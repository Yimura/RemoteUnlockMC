#include <esp_sleep.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "common.hpp"
#include "abstractions/gpio/GPIO.hpp"
#include "abstractions/storage/Storage.hpp"
#include "abstractions/storage/StorageItem.hpp"
#include "abstractions/ble/Ble.hpp"
#include "abstractions/scheduler/Scheduler.hpp"

#include "services/DoorService.hpp"
#include "services/StatusService.hpp"
#include "services/SettingsService.hpp"

#include "LogHelper.hpp"

namespace
{
    constexpr int MAX_BLE_INIT_ATTEMPTS    = 3;
    constexpr uint64_t LOW_POWER_SLEEP_US  = 60ULL * 1000 * 1000;

    [[noreturn]] void EnterLowPowerNoOp()
    {
        // Onboard LED is left RED by DoorService's ctor; it stays as the
        // visual indicator of the no-op state.
        esp_sleep_enable_timer_wakeup(LOW_POWER_SLEEP_US);
        while (true)
        {
            esp_light_sleep_start();
        }
    }
}

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

    bool initialized = false;
    for (int attempt = 1; attempt <= MAX_BLE_INIT_ATTEMPTS && !initialized; ++attempt)
    {
        initialized = g_BleServer.Init();
        if (!initialized)
        {
            LOG(WARNING) << "BLE init failed, attempt " << attempt << "/" << MAX_BLE_INIT_ATTEMPTS;
            g_BleServer.Destroy();
            vTaskDelay(pdMS_TO_TICKS(500 * attempt));
        }
    }

    if (!initialized)
    {
        LOG(FATAL) << "BLE init unrecoverable, entering low-power no-op state";
        EnterLowPowerNoOp();
    }

    g_Scheduler.Start(SCHEDULER_UPDATE_RATE);

    // BLOCKING CALL, DO NOT PUT ANYTHING BELOW THIS, IT WILL NEVER RUN!!!
    g_BleServer.Run();

    g_Storage.Destroy();

    return;
}

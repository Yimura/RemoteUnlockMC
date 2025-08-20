#pragma once

#include <iostream>

#include <AsyncLogger/Logger.hpp>

namespace RemoteUnlock
{
    using namespace al;

    // default stored device name in NVS
    constexpr auto DEFAULT_DEVICE_NAME = "Remote Unlock";
    // update rate in milliseconds
    constexpr auto SCHEDULER_UPDATE_RATE = 500;
} // namespace RemoteUnlock

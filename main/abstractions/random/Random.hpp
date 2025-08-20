#pragma once
#include <esp_random.h>

namespace RemoteUnlock
{
    class Random final
    {
    private:
        Random()          = default;
        virtual ~Random() = default;

    public:
        static float BetweenFloat(float lower, float upper)
        {
            auto random     = esp_random();
            auto normalized = static_cast<float>(random) / UINT32_MAX;
            return lower + normalized * (upper - lower);
        }
    };
} // namespace RemoteUnlock

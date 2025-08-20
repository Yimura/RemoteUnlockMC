#pragma once
#include <functional>
#include <vector>

namespace RemoteUnlock
{
    class Scheduler
    {
    private:
        using Callback = std::function<void(void)>;

        struct Job
        {
            Callback m_Cb;
            uint32_t m_interval_ms;
            int64_t m_next_fire_time_ms = 0;
        };

        std::vector<Job> m_Jobs;
        uint32_t m_UpdateRate;

    public:
        Scheduler()          = default;
        virtual ~Scheduler() = default;

        void AddJob(const Callback& cb, uint32_t interval_ms);
        void Start(uint32_t timer_interval_ms);

    private:
        void Run();
    };

    inline Scheduler g_Scheduler{};
} // namespace RemoteUnlock

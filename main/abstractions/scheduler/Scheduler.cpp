#include "Scheduler.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>

namespace RemoteUnlock
{
    void Scheduler::AddJob(const Callback& cb, uint32_t interval_ms)
    {
        m_Jobs.emplace_back(Job{cb, interval_ms});
    }

    void Scheduler::Start(uint32_t timer_interval_ms)
    {
        m_UpdateRate = timer_interval_ms;

        auto result = xTaskCreate(
            [](void* arg) { static_cast<Scheduler*>(arg)->Run(); }, "InternalScheduler", 4096, this, 5, nullptr);
        if (result != pdPASS)
        {
            LOG(FATAL) << "Failed create scheduled task!";
        }
    }

    void Scheduler::Run()
    {
        LOG(INFO) << "Starting internal runner.";
        while (true)
        {
            int64_t now = esp_timer_get_time();
            for (auto& job : m_Jobs)
            {
                if (job.m_Cb && now >= job.m_next_fire_time_ms)
                {
                    job.m_Cb();

                    job.m_next_fire_time_ms = now + job.m_interval_ms * 1000;
                }
            }
            vTaskDelay(pdMS_TO_TICKS(m_UpdateRate));
        }
    }
} // namespace RemoteUnlock

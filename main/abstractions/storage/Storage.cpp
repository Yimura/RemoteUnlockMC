#include "Storage.hpp"

namespace RemoteUnlock
{
    esp_err_t Storage::Init()
    {
        esp_err_t err = nvs_flash_init();
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            ESP_ERROR_CHECK(nvs_flash_erase());
            err = nvs_flash_init();
        }
        ESP_ERROR_CHECK(err);

        return ESP_OK;
    }

    esp_err_t Storage::Destroy()
    {
        return nvs_flash_deinit();
    }
}

#pragma once
#include <iostream>
#include <optional>
#include <functional>
#include <unordered_map>
#include <string>

// nvs flash storage includes
#include <nvs.h>
#include <nvs_flash.h>

namespace RemoteUnlock
{
    class Storage
    {
    private:
        static constexpr char NAMESPACE[] = "storage";

    public:
        Storage()          = default;
        virtual ~Storage() = default;

        esp_err_t Init();
        esp_err_t Destroy();

        template<typename T>
        T Get(const char* key, T& defaut)
        {
            T val;
            if (!Get<T>(key, &val))
            {
                return defaut;
            }
            return val;
        }

        template<typename T>
        std::optional<T> Get(const char* key)
        {
            T val;
            if (!Get<T>(key, &val))
            {
                return std::nullopt;
            }
            return val;
        }

        template<typename T>
        bool Get(const char* key, T* value)
        {
            nvs_handle_t nvs_handle;
            esp_err_t err = nvs_open(NAMESPACE, NVS_READWRITE, &nvs_handle);
            if (err != ESP_OK)
            {
                return false;
            }

            size_t size = sizeof(T);
            err         = nvs_get_blob(nvs_handle, key, value, &size);
            if (err != ESP_OK)
            {
                return false;
            }

            nvs_close(nvs_handle);
            return true;
        }

        template<typename T>
        bool Set(const char* key, T& value)
        {
            nvs_handle_t nvs_handle;
            esp_err_t err = nvs_open(NAMESPACE, NVS_READWRITE, &nvs_handle);
            if (err != ESP_OK)
            {
                std::cout << "Failed to open NVS handle." << std::endl;
                return false;
            }

            err = nvs_set_blob(nvs_handle, key, &value, sizeof(T));
            if (err != ESP_OK)
            {
                std::cout << "Failed to write value." << std::endl;

                return false;
            }

            err = nvs_commit(nvs_handle);
            if (err != ESP_OK)
            {
                std::cout << "Failed to commit data to NVS." << std::endl;

                return false;
            }

            nvs_close(nvs_handle);
            return true;
        }
    };

    inline Storage g_Storage{};
} // namespace RemoteUnlock

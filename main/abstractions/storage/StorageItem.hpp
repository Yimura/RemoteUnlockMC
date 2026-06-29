#pragma once
#include <cstring>
#include "Storage.hpp"

namespace RemoteUnlock
{
    template<std::size_t N>
    struct StorageItemId
    {
        char m_Key[N];

    public:
        consteval StorageItemId(const char (&id)[N])
        {
            std::ranges::copy(id, m_Key);
        }

        const char* Key() const
        {
            return m_Key;
        }
    };

    template<StorageItemId Id, typename T>
    class StorageItem
    {
    private:
        mutable T m_Value;
        mutable bool m_Cached = false;

    public:
        StorageItem(T defaultValue)
        requires(std::is_copy_constructible_v<T>)
            : m_Value(defaultValue)
        {
        }

        StorageItem(const char* defaultValue)
        {
            for (size_t i = 0; i < sizeof(T); i++)
            {
                m_Value[i] = defaultValue[i];

                if (defaultValue[i] == '\0')
                {
                    break;
                }
            }
        }

        virtual ~StorageItem()                         = default;
        StorageItem(const StorageItem&)                = default;
        StorageItem(StorageItem&&) noexcept            = default;
        StorageItem& operator=(const StorageItem&)     = default;
        StorageItem& operator=(StorageItem&&) noexcept = default;

        const char* Identifier() const
        {
            return Id.Key();
        }

        T& Get() const
        {
            if (!m_Cached)
            {
                g_Storage.Get(Id.Key(), &m_Value);
                m_Cached = true;
            }
            return m_Value;
        }

        bool Set(T& newValue)
        {
            if (!g_Storage.Set(Id.Key(), newValue))
            {
                return false;
            }

            m_Value  = newValue;
            m_Cached = true;
            return true;
        }

        bool Set(const char* new_value)
        {
            const auto length = strlen(new_value);
            if (length >= sizeof(m_Value))
            {
                return false;
            }

            T tmp = {};
            for (size_t i = 0; i < length; i++)
            {
                tmp[i] = new_value[i];
            }

            if (!g_Storage.Set(Id.Key(), tmp))
            {
                return false;
            }

            std::memcpy(m_Value, tmp, sizeof(T));
            m_Cached = true;
            return true;
        }
    };
} // namespace RemoteUnlock

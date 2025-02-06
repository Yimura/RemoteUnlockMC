#pragma once
#include <cstring>
#include <nimble/ble.h>

inline constexpr ble_gatt_svc_def EMPTY_GATT_SVC_DEF()
{
    return {.type = 0};
}

inline constexpr ble_gatt_chr_def EMPTY_GATT_CHR_DEF()
{
    return {.uuid = nullptr};
}

template<typename T>
inline constexpr int MbufAppend(os_mbuf* mbuff, const T& value)
{
    return os_mbuf_append(mbuff, &value, sizeof(T));
}

/**
 * @brief Reads from the Mbuf for an exact amount of data.
 * @tparam T Data type
 * @param mbuff Buffer object
 * @param outvalue Output value
 * @return If the Mbuf is unable to provide that amount this function will return false, true on success.
 */
template<typename T>
inline constexpr bool MbufReadExact(os_mbuf* mbuff, T& outvalue)
{
    if (mbuff->om_len != sizeof(T))
    {
        return false;
    }

    std::memcpy(&outvalue, mbuff->om_data, sizeof(T));
    return true;
}

/**
 * @brief Reads from Mbuf for at least the size of the object.
 * @tparam T Data type
 * @param mbuff Buffer object
 * @param outvalue Output value
 * @return If the Mbuf is smaller than the required size this function will return false, true on success.
 */
template<typename T>
inline constexpr bool MbufReadPartial(os_mbuf* mbuff, T& outvalue)
{
    if (mbuff->om_len < sizeof(T))
    {
        return false;
    }

    std::memcpy(&outvalue, mbuff->om_data, sizeof(T));
    return true;
}

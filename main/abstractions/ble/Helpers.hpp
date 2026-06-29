#pragma once
#include <cstring>
#include <string>
#include <host/ble_hs_mbuf.h>
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
inline bool MbufReadExact(os_mbuf* mbuff, T& outvalue)
{
    if (OS_MBUF_PKTLEN(mbuff) != sizeof(T))
    {
        return false;
    }

    uint16_t copied = 0;
    return ble_hs_mbuf_to_flat(mbuff, &outvalue, sizeof(T), &copied) == 0 && copied == sizeof(T);
}

/**
 * @brief Reads from Mbuf for at least the size of the object.
 * @tparam T Data type
 * @param mbuff Buffer object
 * @param outvalue Output value
 * @return If the Mbuf is smaller than the required size this function will return false, true on success.
 */
template<typename T>
inline bool MbufReadPartial(os_mbuf* mbuff, T& outvalue)
{
    if (OS_MBUF_PKTLEN(mbuff) < sizeof(T))
    {
        return false;
    }

    uint16_t copied = 0;
    return ble_hs_mbuf_to_flat(mbuff, &outvalue, sizeof(T), &copied) == 0 && copied == sizeof(T);
}

inline std::string MbufReadString(os_mbuf* mbuff)
{
    const uint16_t total = OS_MBUF_PKTLEN(mbuff);

    char* data = new char[total];
    uint16_t copied = 0;
    ble_hs_mbuf_to_flat(mbuff, data, total, &copied);

    std::string retvalue(data, copied);

    delete[] data;
    return retvalue;
}

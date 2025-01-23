#pragma once
#include <nimble/ble.h>

inline constexpr ble_gatt_svc_def EMPTY_GATT_SVC_DEF()
{
    return {.type = 0};
}

inline constexpr ble_gatt_chr_def EMPTY_GATT_CHR_DEF()
{
    return {.uuid = nullptr};
}

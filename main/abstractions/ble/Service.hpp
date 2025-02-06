#pragma once
#include <cstdint>
#include <memory>
#include <vector>

#include <host/ble_gatt.h>
#include <host/ble_uuid.h>

#include "Helpers.hpp"

namespace RemoteUnlock
{
    class BleCharacteristic;

    class BleService
    {
    private:
        ble_uuid_any_t m_Uuid;
        uint8_t m_Type;
        std::vector<BleCharacteristic*> m_Characteristics;

        // internal ref characteristic array
        std::unique_ptr<ble_gatt_chr_def[]> m_InternalCharacteristic;

    public:
        BleService(ble_uuid_any_t uuid, uint8_t type);
        ble_gatt_svc_def Build();
        void RegisterCharacteristic(BleCharacteristic& characteristics);
    };
} // namespace RemoteUnlock

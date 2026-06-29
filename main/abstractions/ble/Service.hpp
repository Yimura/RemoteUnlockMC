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

        /**
         * @brief Registers the given characteristics on this service and then registers the service with the BLE
         * server in a single call. Coupling the two steps keeps a newly declared characteristic from silently
         * disappearing because someone forgot to add the corresponding registration line.
         */
        template<typename... Chrs>
        void Register(Chrs&... characteristics)
        {
            (RegisterCharacteristic(characteristics), ...);
            RegisterSelf();
        }

    private:
        void RegisterCharacteristic(BleCharacteristic& characteristic);
        void RegisterSelf();
    };
} // namespace RemoteUnlock

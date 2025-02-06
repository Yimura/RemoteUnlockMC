#include "Service.hpp"

#include "Characteristic.hpp"
#include "Helpers.hpp"

namespace RemoteUnlock
{
    BleService::BleService(ble_uuid_any_t uuid, uint8_t type) : m_Uuid(uuid), m_Type(type)
    {
    }

    ble_gatt_svc_def BleService::Build()
    {
        // + 1 is to include the empty definition (indicates end of characteristics array)
        const std::size_t count  = m_Characteristics.size();
        m_InternalCharacteristic = std::make_unique<ble_gatt_chr_def[]>(count + 1);

        for (int i = 0; i < count; ++i)
        {
            m_InternalCharacteristic[i] = m_Characteristics[i]->Build();
        }
        m_InternalCharacteristic[count] = EMPTY_GATT_CHR_DEF();
        return ble_gatt_svc_def{
            .type            = m_Type,
            .uuid            = &m_Uuid.u,
            .characteristics = m_InternalCharacteristic.get(),
        };
    }

    void BleService::RegisterCharacteristic(BleCharacteristic& characteristic)
    {
        m_Characteristics.push_back(&characteristic);
    }
} // namespace RemoteUnlock

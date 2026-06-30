#pragma once
#include <cstdint>
#include <functional>

#include <host/ble_gatt.h>
#include <host/ble_uuid.h>

namespace RemoteUnlock
{
    using BleChrAccessCb =
        std::function<int(uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg)>;

    class BleCharacteristic
    {
    private:
        ble_uuid_any_t m_Uuid;
        uint16_t m_Flags;
        BleChrAccessCb m_Callback;
        uint16_t m_ValueHandle;

    public:
        BleCharacteristic(ble_uuid_any_t uuid, uint16_t flags, BleChrAccessCb access_callback);
        virtual ~BleCharacteristic() = default;

        ble_gatt_chr_def Build();
        /**
         * @brief Indicates to a connection that a new value can be read.
         * @param conn_handle Handle of the connection to indicate.
         * @return true on success, false otherwise.
         */
        bool Indicate(uint16_t conn_handle);

        /**
         * @brief Broadcasts the new value to every subscribed central via
         *        notify or indicate, picked per their CCCD configuration.
         *        Use this when a value updates outside the access callback
         *        (e.g. a scheduler-driven sensor refresh) — no conn_handle
         *        is needed and disconnected peers are silently skipped.
         */
        void IndicateAll();

    private:
        static int CharacteristicAccessCallback(
            uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg);
    };
} // namespace RemoteUnlock

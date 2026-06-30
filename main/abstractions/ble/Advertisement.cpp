#include "Ble.hpp"
#include "services/ServiceDefinitions.hpp"

#include <nimble/hci_common.h>
#include <os/os_mbuf.h>

namespace RemoteUnlock
{
    uint8_t g_OwnerAddrType;
    uint8_t g_AddrVal[6] = {0};

    bool Ble::AdvertisementInit()
    {
        int rc;

        rc = ble_hs_util_ensure_addr(0);
        if (rc != 0)
        {
            LOG(FATAL) << "Device does not have any available bluetooth address configured!";

            return false;
        }

        rc = ble_hs_id_infer_auto(0, &g_OwnerAddrType);
        if (rc != 0)
        {
            LOG(FATAL) << "Failed to infer address type, error code: " << rc;

            return false;
        }

        rc = ble_hs_id_copy_addr(g_OwnerAddrType, g_AddrVal, nullptr);
        if (rc != 0)
        {
            LOG(FATAL) << "Failed to copy device address, error code: " << rc;

            return false;
        }

        AdvertisementStart();
        return true;
    }

    constexpr auto BLE_GAP_LE_ROLE_PERIPHERAL                = 0x0;
    // Set to 0x0000 (Generic) during pairing debug so iOS does not engage its
    // HID-over-GATT special handling. Restore 0x0180 (Generic Remote Control) once
    // pairing is stable.
    constexpr auto BLE_GAP_APPEARANCE_GENERIC_REMOTE_CONTROL = 0x0000;
    constexpr uint8_t EXT_ADV_INSTANCE                       = 0;
    constexpr size_t EXT_ADV_DATA_MAX                        = 255;

    bool Ble::AdvertisementStart()
    {
        if (ble_gap_ext_adv_active(EXT_ADV_INSTANCE))
        {
            return true;
        }

        ble_gap_ext_adv_params params = {};
        params.connectable            = 1;
        params.scannable              = 0;
        params.legacy_pdu             = 0;
        params.include_tx_power       = 1;
        params.own_addr_type          = g_OwnerAddrType;
        params.primary_phy            = BLE_HCI_LE_PHY_1M;
        params.secondary_phy          = BLE_HCI_LE_PHY_2M;
        params.tx_power               = 127;
        params.sid                    = 0;
        // Hot advert cadence: 200 ms. At 500 ms the Android central's
        // BALANCED/LOW_POWER scan window only catches one PDU per scan slot
        // and first-hit while approaching the vehicle took up to a minute,
        // making CONFIRM mode unusably slow. 200 ms triples the hit rate
        // for a modest power increase the peripheral can absorb.
        params.itvl_min               = BLE_GAP_ADV_ITVL_MS(200);
        params.itvl_max               = BLE_GAP_ADV_ITVL_MS(210);

        int8_t selected_tx_power = 0;
        int rc                   = ble_gap_ext_adv_configure(EXT_ADV_INSTANCE, &params, &selected_tx_power,
                              [](ble_gap_event* event, void* args) -> int { return g_BleServer.GapEventHandler(event, args); },
                              nullptr);
        if (rc != 0)
        {
            LOG(FATAL) << "ble_gap_ext_adv_configure failed: " << rc;
            return false;
        }

        ble_hs_adv_fields fields = {0};
        fields.flags             = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

        const char* name        = ble_svc_gap_device_name();
        fields.name             = reinterpret_cast<uint8_t*>(const_cast<char*>(name));
        fields.name_len         = strlen(name);
        fields.name_is_complete = 1;

        // Skip explicit AD tx-power field. With ext adv on a legacy-controller
        // target the host's HCI Read Advertising Channel TX Power returns
        // CMD_DISALLOWED (0x0C), failing ble_hs_adv_set_fields. Controller
        // already tracks per-instance TX power set via params.tx_power.

        fields.appearance            = BLE_GAP_APPEARANCE_GENERIC_REMOTE_CONTROL;
        fields.appearance_is_present = 1;

        fields.le_role            = BLE_GAP_LE_ROLE_PERIPHERAL;
        fields.le_role_is_present = 1;

        fields.uuids128             = &DoorServiceUUID;
        fields.num_uuids128         = 1;
        fields.uuids128_is_complete = true;

        fields.device_addr            = g_AddrVal;
        fields.device_addr_type       = g_OwnerAddrType;
        fields.device_addr_is_present = 1;

        fields.adv_itvl            = BLE_GAP_ADV_ITVL_MS(200);
        fields.adv_itvl_is_present = 1;

        uint8_t buf[EXT_ADV_DATA_MAX];
        uint8_t buf_len = 0;
        rc              = ble_hs_adv_set_fields(&fields, buf, &buf_len, EXT_ADV_DATA_MAX);
        if (rc != 0)
        {
            LOG(FATAL) << "ble_hs_adv_set_fields failed: " << rc;
            return false;
        }

        os_mbuf* mbuf = os_msys_get_pkthdr(buf_len, 0);
        if (!mbuf)
        {
            LOG(FATAL) << "os_msys_get_pkthdr returned null";
            return false;
        }
        rc = os_mbuf_append(mbuf, buf, buf_len);
        if (rc != 0)
        {
            os_mbuf_free_chain(mbuf);
            LOG(FATAL) << "os_mbuf_append failed: " << rc;
            return false;
        }

        rc = ble_gap_ext_adv_set_data(EXT_ADV_INSTANCE, mbuf);
        if (rc != 0)
        {
            LOG(FATAL) << "ble_gap_ext_adv_set_data failed: " << rc;
            return false;
        }

        rc = ble_gap_ext_adv_start(EXT_ADV_INSTANCE, 0, 0);
        if (rc != 0)
        {
            LOG(FATAL) << "ble_gap_ext_adv_start failed: " << rc;
            return false;
        }

        LOG(INFO) << "Advertised!";
        return true;
    }

} // namespace RemoteUnlock

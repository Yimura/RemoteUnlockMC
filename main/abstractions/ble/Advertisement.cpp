#include "Ble.hpp"

namespace RemoteUnlock
{
    uint8_t OWNER_ADDR_TYPE;
    uint8_t ADDR_VAL[6] = {0};

    bool Ble::AdvertisementInit()
    {
        int rc;

        rc = ble_hs_util_ensure_addr(0);
        if (rc != 0)
        {
            std::cout << "Device does not have any available bluetooth address configured!" << std::endl;

            return false;
        }

        rc = ble_hs_id_infer_auto(0, &OWNER_ADDR_TYPE);
        if (rc != 0)
        {
            std::cout << "Failed to infer address type, error code: " << rc << std::endl;

            return false;
        }

        rc = ble_hs_id_copy_addr(OWNER_ADDR_TYPE, ADDR_VAL, nullptr);
        if (rc != 0)
        {
            std::cout << "Failed to copy device address, error code: " << rc << std::endl;

            return false;
        }

        AdvertisementStart();
        return true;
    }

    constexpr auto BLE_GAP_LE_ROLE_PERIPHERAL                = 0x0;
    constexpr auto BLE_GAP_APPEARANCE_GENERIC_REMOTE_CONTROL = 0x0180;

    bool Ble::AdvertisementStart()
    {
        struct ble_hs_adv_fields adv_fields  = {0};
        struct ble_hs_adv_fields rsp_fields  = {0};
        struct ble_gap_adv_params adv_params = {0};

        /* Set advertising flags */
        adv_fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

        /* Set device name */
        const char* name            = ble_svc_gap_device_name();
        adv_fields.name             = (uint8_t*)name;
        adv_fields.name_len         = strlen(name);
        adv_fields.name_is_complete = 1;

        /* Set device tx power */
        adv_fields.tx_pwr_lvl            = BLE_HS_ADV_TX_PWR_LVL_AUTO;
        adv_fields.tx_pwr_lvl_is_present = 1;

        /* Set device appearance */
        adv_fields.appearance            = BLE_GAP_APPEARANCE_GENERIC_REMOTE_CONTROL;
        adv_fields.appearance_is_present = 1;

        /* Set device LE role */
        adv_fields.le_role            = BLE_GAP_LE_ROLE_PERIPHERAL;
        adv_fields.le_role_is_present = 1;

        /* Set advertiement fields */
        int rc = ble_gap_adv_set_fields(&adv_fields);
        if (rc != 0)
        {
            std::cout << "failed to set advertising data, error code: " << rc << std::endl;
            return false;
        }

        /* Set device address */
        rsp_fields.device_addr            = ADDR_VAL;
        rsp_fields.device_addr_type       = OWNER_ADDR_TYPE;
        rsp_fields.device_addr_is_present = 1;

        /* Set URI */
        // rsp_fields.uri     = esp_uri;
        // rsp_fields.uri_len = sizeof(esp_uri);

        /* Set advertising interval */
        rsp_fields.adv_itvl            = BLE_GAP_ADV_ITVL_MS(500);
        rsp_fields.adv_itvl_is_present = 1;

        /* Set scan response fields */
        rc = ble_gap_adv_rsp_set_fields(&rsp_fields);
        if (rc != 0)
        {
            std::cout << "failed to set scan response data, error code: " << rc << std::endl;
            return false;
        }

        /* Set non-connetable and general discoverable mode to be a beacon */
        adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
        adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

        /* Set advertising interval */
        adv_params.itvl_min = BLE_GAP_ADV_ITVL_MS(500);
        adv_params.itvl_max = BLE_GAP_ADV_ITVL_MS(510);

        /* Start advertising */
        rc = ble_gap_adv_start(OWNER_ADDR_TYPE, NULL, BLE_HS_FOREVER, &adv_params,
            [](ble_gap_event* event, void* args) -> int { return g_BleServer.GapEventHandler(event, args); }, NULL);
        if (rc != 0)
        {
            std::cout << "failed to start advertising, error code: " << rc << std::endl;
            return false;
        }

        std::cout << "Advertised!" << std::endl;
        return true;
    }

} // namespace RemoteUnlock

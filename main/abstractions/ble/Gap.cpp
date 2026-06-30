#include "Ble.hpp"

#include <cinttypes>
#include <cstdio>

namespace
{
    // Format a 6-byte BLE address into a stable AA:BB:CC:DD:EE:FF string for
    // logging. NimBLE stores addresses little-endian (byte 0 = LSB), so render
    // in reverse to match the canonical form printed by Android / nRF Connect.
    void FormatAddr(const uint8_t addr[6], uint8_t type, char out[32])
    {
        const char* tag = type == BLE_OWN_ADDR_PUBLIC             ? "pub"
                        : type == BLE_OWN_ADDR_RANDOM             ? "rnd"
                        : type == BLE_OWN_ADDR_RPA_PUBLIC_DEFAULT ? "rpa-pub"
                        : type == BLE_OWN_ADDR_RPA_RANDOM_DEFAULT ? "rpa-rnd"
                                                                  : "?";
        std::snprintf(out, 32, "%02X:%02X:%02X:%02X:%02X:%02X/%s",
                      addr[5], addr[4], addr[3], addr[2], addr[1], addr[0], tag);
    }
}

namespace RemoteUnlock
{
    bool RemoteUnlock::Ble::GapInit()
    {
        ble_svc_gap_init();
        if (ble_svc_gap_device_name_set(GetDeviceName()) != 0)
        {
            return false;
        }
        return true;
    }

    int Ble::GapEventHandler(ble_gap_event* event, void* args)
    {
        switch (event->type)
        {
        case BLE_GAP_EVENT_CONNECT:
            return GapEventConnect(event);
        case BLE_GAP_EVENT_DISCONNECT:
            return GapEventDisconnect(event);
        case BLE_GAP_EVENT_CONN_UPDATE:
            return GapEventConnUpdate(event);
        case BLE_GAP_EVENT_ADV_COMPLETE:
            return GapEventAdvertisementComplete(event);
        case BLE_GAP_EVENT_SUBSCRIBE:
            return GapEventSubscribe(event);
        case BLE_GAP_EVENT_MTU:
            return GapEventMtuUpdate(event);
        case BLE_GAP_EVENT_PASSKEY_ACTION:
            return GapEventPasskeyAction(event);
        case BLE_GAP_EVENT_ENC_CHANGE:
            return GapEventEncryptionChange(event);
        case BLE_GAP_EVENT_REPEAT_PAIRING:
            return GapEventRepeatPairing(event);
        case BLE_GAP_EVENT_IDENTITY_RESOLVED:
            return GapEventIdentityResolved(event);
        }
        return 0;
    }

    int Ble::GapEventConnect(ble_gap_event* event)
    {
        LOG(VERBOSE) << "New connection was established";
        if (event->connect.status != 0)
        {
            LOG(VERBOSE) << "Connection failure, restarting advertisement.";
            AdvertisementStart();
            return 0;
        }

        ble_gap_conn_desc desc;
        if (int rc = ble_gap_conn_find(event->connect.conn_handle, &desc); rc != 0)
        {
            LOG(WARNING) << "failed to find connection by handle, err code: " << rc;

            return rc;
        }

        // Clear any leftover passkey marker for this handle so a later
        // BLE_GAP_EVENT_ENC_CHANGE on the same handle can correctly
        // distinguish REUSED-BOND from FRESH-PAIRING.
        m_PairingConnHandles.erase(event->connect.conn_handle);

        char peer_ota[32], peer_id[32];
        FormatAddr(desc.peer_ota_addr.val, desc.peer_ota_addr.type, peer_ota);
        FormatAddr(desc.peer_id_addr.val, desc.peer_id_addr.type, peer_id);
        LOG(INFO) << "GAP CONNECT conn=" << desc.conn_handle
                  << " peer_ota=" << peer_ota
                  << " peer_id=" << peer_id
                  << " encrypted=" << (int)desc.sec_state.encrypted
                  << " authenticated=" << (int)desc.sec_state.authenticated
                  << " bonded=" << (int)desc.sec_state.bonded;

        ble_gap_upd_params params = {
            .itvl_min            = desc.conn_itvl,
            .itvl_max            = desc.conn_itvl,
            .latency             = 3,
            .supervision_timeout = desc.supervision_timeout,
        };

        int rc = ble_gap_update_params(event->connect.conn_handle, &params);
        if (rc != 0)
        {
            LOG(WARNING) << "failed to update connection params, error code: " << rc;
        }
        return rc;
    }

    int Ble::GapEventDisconnect(ble_gap_event* event)
    {
        LOG(INFO) << "disconnected from peer, reason: " << event->disconnect.reason;

        m_PairingConnHandles.erase(event->disconnect.conn.conn_handle);

        AdvertisementStart();
        return 0;
    }

    int Ble::GapEventConnUpdate(ble_gap_event* event)
    {
        LOG(VERBOSE) << "Received connection update.";

        return 0;
    }

    int Ble::GapEventAdvertisementComplete(ble_gap_event* event)
    {
        LOG(VERBOSE) << "advertisement complete";

        AdvertisementStart();
        return 0;
    }

    int Ble::GapEventSubscribe(ble_gap_event* event)
    {
        return 0;
    }

    int Ble::GapEventMtuUpdate(ble_gap_event* event)
    {
        LOG(VERBOSE) << "MTU update vent; conn_handle=" << event->mtu.conn_handle << " cid=" << event->mtu.channel_id
                     << " mtu=" << event->mtu.value;

        return 0;
    }

    int Ble::GapEventPasskeyAction(ble_gap_event* event)
    {
        // BLE_SM_IOACT_*: 0=NONE 1=OOB 2=INPUT 3=DISP 4=NUMCMP 5=OOB_SC 6=STATIC
        const uint8_t action = event->passkey.params.action;
        // Record this handle so the matching BLE_GAP_EVENT_ENC_CHANGE knows
        // that this encryption activation came from a fresh SMP exchange and
        // not from a stored LTK lookup against the resolving list.
        m_PairingConnHandles.insert(event->passkey.conn_handle);
        LOG(INFO) << "FRESH-PAIRING passkey action=" << (int)action
                  << " conn=" << event->passkey.conn_handle
                  << " numcmp=" << event->passkey.params.numcmp;

        ble_sm_io io = {};
        switch (action)
        {
        case BLE_SM_IOACT_DISP:
        case BLE_SM_IOACT_INPUT:
            io.action  = action;
            io.passkey = BLE_FIXED_PASSKEY;
            break;
        case BLE_SM_IOACT_NUMCMP:
            io.action        = action;
            io.numcmp_accept = 1;
            LOG(INFO) << "auto-accepting numeric compare value " << event->passkey.params.numcmp;
            break;
        default:
            LOG(WARNING) << "unsupported passkey action: " << (int)action;
            return 0;
        }

        int rc = ble_sm_inject_io(event->passkey.conn_handle, &io);
        if (rc != 0)
        {
            LOG(WARNING) << "ble_sm_inject_io failed: " << rc;
        }
        return rc;
    }

    int Ble::GapEventEncryptionChange(ble_gap_event* event)
    {
        const uint16_t conn = event->enc_change.conn_handle;
        const int status    = event->enc_change.status;
        const bool fresh    = m_PairingConnHandles.erase(conn) > 0;
        const char* kind    = fresh ? "FRESH-PAIRING" : "REUSED-BOND";

        ble_gap_conn_desc desc{};
        const int find_rc = ble_gap_conn_find(conn, &desc);
        if (find_rc != 0)
        {
            LOG(INFO) << kind << " ENC-CHANGE conn=" << conn << " status=" << status
                      << " (conn_find rc=" << find_rc << ")";
            return 0;
        }

        char peer_ota[32], peer_id[32];
        FormatAddr(desc.peer_ota_addr.val, desc.peer_ota_addr.type, peer_ota);
        FormatAddr(desc.peer_id_addr.val, desc.peer_id_addr.type, peer_id);
        LOG(INFO) << kind << " ENC-CHANGE conn=" << conn
                  << " status=" << status
                  << " peer_ota=" << peer_ota
                  << " peer_id=" << peer_id
                  << " encrypted=" << (int)desc.sec_state.encrypted
                  << " authenticated=" << (int)desc.sec_state.authenticated
                  << " bonded=" << (int)desc.sec_state.bonded
                  << " key_size=" << (int)desc.sec_state.key_size;
        return 0;
    }

    int Ble::GapEventRepeatPairing(ble_gap_event* event)
    {
        // Fires when the central re-initiates pairing on a link where the
        // peripheral already holds a bond for the peer identity. With
        // CONFIG_BT_NIMBLE_HANDLE_REPEAT_PAIRING_DELETION=y NimBLE will purge
        // the old bond and let the new pairing succeed; otherwise we return
        // BLE_GAP_REPEAT_PAIRING_RETRY ourselves after deleting the bond.
        ble_gap_conn_desc desc{};
        if (ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc) == 0)
        {
            char peer_id[24];
            FormatAddr(desc.peer_id_addr.val, desc.peer_id_addr.type, peer_id);
            LOG(WARNING) << "REPEAT-PAIRING conn=" << event->repeat_pairing.conn_handle
                         << " peer_id=" << peer_id
                         << " — deleting stale bond and accepting fresh pair";
            ble_store_util_delete_peer(&desc.peer_id_addr);
        }
        else
        {
            LOG(WARNING) << "REPEAT-PAIRING conn=" << event->repeat_pairing.conn_handle
                         << " (conn_find failed)";
        }
        return BLE_GAP_REPEAT_PAIRING_RETRY;
    }

    int Ble::GapEventIdentityResolved(ble_gap_event* event)
    {
        // Fires after the controller resolves a peer's RPA against an IRK
        // held in the resolving list. Seeing this is positive proof that
        // IRK exchange + HS_PVCY are working as intended.
        ble_gap_conn_desc desc{};
        if (ble_gap_conn_find(event->identity_resolved.conn_handle, &desc) == 0)
        {
            char peer_ota[32], peer_id[32];
            FormatAddr(desc.peer_ota_addr.val, desc.peer_ota_addr.type, peer_ota);
            FormatAddr(desc.peer_id_addr.val, desc.peer_id_addr.type, peer_id);
            LOG(INFO) << "IDENTITY-RESOLVED conn=" << event->identity_resolved.conn_handle
                      << " rpa=" << peer_ota << " -> identity=" << peer_id;
        }
        return 0;
    }
} // namespace RemoteUnlock

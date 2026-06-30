#include "Ble.hpp"
#include "Characteristic.hpp"
#include "Service.hpp"
#include "Helpers.hpp"

#include <host/ble_sm.h>
#include <host/ble_store.h>

extern "C" void ble_store_config_init(void);

namespace RemoteUnlock
{
    namespace
    {
        // Round-robin overflow handler covering every bond-store object type,
        // not only OUR_SEC/PEER_SEC/CCCD as ble_store_util_status_rr does.
        // ESP-IDF 6.0 NimBLE introduces CSFC + RPA_REC slots (the latter
        // required by CONFIG_BT_NIMBLE_HS_PVCY=y); when those overflow the
        // default handler returns BLE_HS_EUNKNOWN, which propagates as rc=27
        // (BLE_HS_ESTORE_CAP) from the write path. NimBLE then auto-calls
        // ble_gap_unpair, wiping the freshly-established bond from RAM as
        // well — the symptom we hit where pairing succeeds, then the next
        // reconnect fails with ENC status=13.
        int StoreStatusEvictOldest(struct ble_store_status_event* event, void* /*arg*/)
        {
            if (event->event_code != BLE_STORE_EVENT_OVERFLOW)
            {
                return BLE_HS_EUNKNOWN;
            }
            const int rc = ble_store_util_delete_oldest_peer();
            LOG(WARNING) << "bond store overflow obj_type=" << (int)event->overflow.obj_type
                         << " evicted oldest peer rc=" << rc;
            return rc;
        }
    } // namespace

    Ble::Ble() = default;

    void Ble::OnStackSync()
    {
        LOG(VERBOSE) << "Stack Syncronized, starting advertisement init.";
        AdvertisementStart();
    }

    void Ble::OnStackErr(int reason)
    {
        LOG(VERBOSE) << "Stack error with reason: " << reason;
    }

    void Ble::Destroy()
    {
        if (!m_PortInitialized)
        {
            return;
        }
        nimble_port_stop();
        nimble_port_deinit();
        m_PortInitialized = false;
    }

    bool Ble::Init()
    {
        if (m_PortInitialized)
        {
            return true;
        }

        esp_err_t result = nimble_port_init();
        if (result != ESP_OK)
        {
            LOG(WARNING) << "nimble_port_init failed: " << result;
            return false;
        }
        m_PortInitialized = true;

        if (!GapInit() || !GattInit())
        {
            nimble_port_deinit();
            m_PortInitialized = false;
            return false;
        }

        NimbleHostConfigInit();
        return true;
    }

    void Ble::Run()
    {
        LOG(INFO) << "Starting BLE server";

        nimble_port_run();
    }

    const char* Ble::GetDeviceName()
    {
        return m_DeviceName.Get();
    }

    bool Ble::SetDeviceName(const char* new_name)
    {
        if (m_DeviceName.Set(new_name))
        {
            ble_svc_gap_device_name_set(m_DeviceName.Get());
            LOG(VERBOSE) << "Set device name to: " << m_DeviceName.Get();

            return true;
        }
        return false;
    }

    void Ble::RegisterService(BleService& service)
    {
        m_Services.push_back(service.Build());
    }

    void Ble::NimbleHostConfigInit()
    {
        ble_hs_cfg.gatts_register_cb = [](ble_gatt_register_ctxt* ctxt, void* arg) {
            g_BleServer.GattSvrRegisterCb(ctxt, arg);
        };
        ble_hs_cfg.sync_cb         = []() { g_BleServer.OnStackSync(); };
        ble_hs_cfg.reset_cb        = [](int reason) { g_BleServer.OnStackErr(reason); };
        ble_hs_cfg.store_status_cb = StoreStatusEvictOldest;

        // LE Secure Connections with bonding + MITM via fixed display-only passkey.
        // Central enters BLE_FIXED_PASSKEY; CONFIG_BT_NIMBLE_SM_SC_ONLY=1 refuses legacy.
        ble_hs_cfg.sm_io_cap         = BLE_HS_IO_DISPLAY_ONLY;
        ble_hs_cfg.sm_bonding        = 1;
        ble_hs_cfg.sm_mitm           = 1;
        ble_hs_cfg.sm_sc             = 1;
        // Distribute + request IRK in addition to LTK. Without ID-key
        // exchange, the bond store is keyed by the central's resolvable
        // private address; as soon as the phone rotates its RPA (or a new
        // GATT context appears with a different RPA), the peripheral can no
        // longer locate the stored LTK and forces a fresh LESC pairing —
        // surfacing the passkey dialog on every reconnect. Sharing IRKs lets
        // the host's resolving list translate any future RPA back to the
        // bonded identity so the stored LTK is reused silently.
        ble_hs_cfg.sm_our_key_dist   = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
        ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;

        ble_store_config_init();
    }
} // namespace RemoteUnlock

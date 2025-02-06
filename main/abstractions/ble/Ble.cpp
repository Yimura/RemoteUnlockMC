#include "Ble.hpp"
#include "Characteristic.hpp"
#include "Service.hpp"
#include "Helpers.hpp"

extern "C" void ble_store_config_init(void);

namespace RemoteUnlock
{
    Ble::Ble() : m_GapEventHandlers(5)
    {
        RegisterEventHandlers();
    }

    void Ble::GattSvrRegisterCb(ble_gatt_register_ctxt* ctxt, void* arg)
    {
        char bleUUID[BLE_UUID_STR_LEN];

        switch (ctxt->op)
        {
        case BLE_GATT_REGISTER_OP_SVC:
            std::cout << "Registered bluetooth service " << ble_uuid_to_str(ctxt->svc.svc_def->uuid, bleUUID)
                      << " with handle=" << ctxt->svc.handle << std::endl;

            break;

        case BLE_GATT_REGISTER_OP_CHR:
            std::cout << "Registered bluetooth characteristic " << ble_uuid_to_str(ctxt->chr.chr_def->uuid, bleUUID)
                      << " with handle=" << ctxt->chr.def_handle << std::endl;

            break;

        case BLE_GATT_REGISTER_OP_DSC:
            std::cout << "Registered bluetooth descriptor " << ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, bleUUID)
                      << " with handle=" << ctxt->dsc.handle << std::endl;

            break;

        default:
            std::cout << "Unknown event caught: " << ctxt->op << std::endl;

            break;
        }
    }

    void Ble::OnStackSync()
    {
        std::cout << "Stack Syncronized, starting advertisement init." << std::endl;
        AdvertisementStart();
    }

    void Ble::OnStackErr(int reason)
    {
        std::cout << "Stack error with reason: " << reason << std::endl;
    }

    void Ble::Destroy()
    {
        nimble_port_stop();
    }

    bool Ble::Init()
    {
        esp_err_t result = nimble_port_init();
        if (result != ESP_OK)
        {
            return false;
        }

        // I sincerely hope call order is preserved, plz complier
        if (!GapInit() || !GattInit())
        {
            return false;
        }

        NimbleHostConfigInit();
        return true;
    }

    void Ble::Run()
    {
        std::cout << "Starting BLE server" << std::endl;

        nimble_port_run();
    }

    void Ble::RegisterService(BleService& service)
    {
        m_Services.push_back(service.Build());
    }

    void Ble::RegisterEventHandlers()
    {
        m_GapEventHandlers[BLE_GAP_EVENT_CONNECT] = [](ble_gap_event* event) {
            return g_BleServer.GapEventConnect(event);
        };
        m_GapEventHandlers[BLE_GAP_EVENT_DISCONNECT] = [](ble_gap_event* event) {
            return g_BleServer.GapEventDisconnect(event);
        };
        m_GapEventHandlers[BLE_GAP_EVENT_CONN_UPDATE] = [](ble_gap_event* event) {
            return g_BleServer.GapEventConnUpdate(event);
        };
        m_GapEventHandlers[BLE_GAP_EVENT_ADV_COMPLETE] = [](ble_gap_event* event) {
            return g_BleServer.GapEventAdvertisementComplete(event);
        };
        m_GapEventHandlers[BLE_GAP_EVENT_SUBSCRIBE] = [](ble_gap_event* event) {
            return g_BleServer.GapEventSubscribe(event);
        };
        m_GapEventHandlers[BLE_GAP_EVENT_MTU] = [](ble_gap_event* event) {
            return g_BleServer.GapEventMtuUpdate(event);
        };
    }

    void Ble::NimbleHostConfigInit()
    {
        ble_hs_cfg.gatts_register_cb = [](ble_gatt_register_ctxt* ctxt, void* arg) {
            g_BleServer.GattSvrRegisterCb(ctxt, arg);
        };
        ble_hs_cfg.sync_cb         = []() { g_BleServer.OnStackSync(); };
        ble_hs_cfg.reset_cb        = [](int reason) { g_BleServer.OnStackErr(reason); };
        ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

        ble_store_config_init();
    }
} // namespace RemoteUnlock

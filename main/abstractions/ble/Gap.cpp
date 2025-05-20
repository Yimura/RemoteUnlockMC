#include "Ble.hpp"

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
        if (auto it = m_GapEventHandlers.find(event->type); it != m_GapEventHandlers.end())
        {
            return it->second(event);
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
} // namespace RemoteUnlock

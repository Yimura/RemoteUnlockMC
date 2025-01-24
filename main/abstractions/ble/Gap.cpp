#include "Ble.hpp"

namespace RemoteUnlock
{
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
        std::cout << "New connection was established" << std::endl;
        if (event->connect.status != 0)
        {
            std::cout << "Connection failure, restarting advertisement." << std::endl;
            AdvertisementStart();
            return 0;
        }

        ble_gap_conn_desc desc;
        if (int rc = ble_gap_conn_find(event->connect.conn_handle, &desc); rc != 0)
        {
            std::cout << "failed to find connection by handle, err code: " << rc << std::endl;

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
            std::cout << "failed to update connection params, error code: " << rc << std::endl;
        }
        return rc;
    }

    int Ble::GapEventDisconnect(ble_gap_event* event)
    {
        std::cout << "disconnected from peer, reason: " << event->disconnect.reason << std::endl;

        AdvertisementStart();
        return 0;
    }

    int Ble::GapEventConnUpdate(ble_gap_event* event)
    {
        std::cout << "Received connection update." << std::endl;

        return 0;
    }

    int Ble::GapEventAdvertisementComplete(ble_gap_event* event)
    {
        std::cout << "advertisement complete" << std::endl;

        AdvertisementStart();
        return 0;
    }

    int Ble::GapEventSubscribe(ble_gap_event* event)
    {
        return 0;
    }

    int Ble::GapEventMtuUpdate(ble_gap_event* event)
    {
        return 0;
    }
} // namespace RemoteUnlock

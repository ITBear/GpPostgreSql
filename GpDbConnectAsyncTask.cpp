#include "GpDbConnectAsyncTask.hpp"

namespace GPlatform {

GpDbConnectAsyncTask::GpDbConnectAsyncTask
(
    std::string_view        aName,
    GpIOEventPoller::WP     aIOPoller,
    PGconn*                 aPGconn,
    GpTaskFiberBarrier::SP  aTaskBarrier
):
GpSocketTask
(
    aName,
    aIOPoller,
    GpSocketTCP::SFromID(PQsocket(aPGconn), GpSocket::CloseModeT::NO_CLOSE_ON_DESTRUCT, GpSocketTCP::StateT::INCOMING)
),
iPGconn(aPGconn),
iTaskBarrier(std::move(aTaskBarrier))
{
}

GpDbConnectAsyncTask::~GpDbConnectAsyncTask (void) noexcept
{
    if (iTaskBarrier.IsNotNULL())
    {
        iTaskBarrier->Release(std::nullopt);
        iTaskBarrier.Clear();
    }
}

GpTask::ResT    GpDbConnectAsyncTask::OnSockReadyToRead (GpSocket& /*aSocket*/)
{
    return ProcessRW();
}

GpTask::ResT    GpDbConnectAsyncTask::OnSockReadyToWrite (GpSocket& /*aSocket*/)
{
    return ProcessRW();
}

void    GpDbConnectAsyncTask::OnSockClosed (GpSocket& /*aSocket*/)
{
    if (iTaskBarrier.IsNotNULL())
    {
        iTaskBarrier->Release(GpDbConnectionStatus(GpDbConnectionStatus::CLOSED));
        iTaskBarrier.Clear();
    }
}

void    GpDbConnectAsyncTask::OnSockError (GpSocket& /*aSocket*/)
{
    if (iTaskBarrier.IsNotNULL())
    {
        iTaskBarrier->Release(GpDbConnectionStatus(GpDbConnectionStatus::CLOSED));
        iTaskBarrier.Clear();
    }
}

GpTask::ResT    GpDbConnectAsyncTask::ProcessRW (void)
{
    PostgresPollingStatusType pgPoolRes = PQconnectPoll(iPGconn);

    if (pgPoolRes == PGRES_POLLING_FAILED)
    {
        if (iTaskBarrier.IsNotNULL())
        {
            iTaskBarrier->Release(GpDbConnectionStatus(GpDbConnectionStatus::CLOSED));
            iTaskBarrier.Clear();
        }

        return GpTask::ResT::DONE;
    } else if (pgPoolRes == PGRES_POLLING_OK)
    {
        if (iTaskBarrier.IsNotNULL())
        {
            iTaskBarrier->Release(GpDbConnectionStatus(GpDbConnectionStatus::CONNECTED));
            iTaskBarrier.Clear();
        }

        return GpTask::ResT::DONE;
    }

    PQconnectPoll(iPGconn);

    //Return to waiting
    return GpTask::ResT::WAITING;
}

}//namespace GPlatform

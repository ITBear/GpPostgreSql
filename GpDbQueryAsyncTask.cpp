#include "GpDbQueryAsyncTask.hpp"
#include "GpDbQueryResPgSql.hpp"

#include <iostream>

namespace GPlatform {

GpDbQueryAsyncTask::GpDbQueryAsyncTask
(
    std::string_view        aName,
    GpDbConnectionPgSql::SP aDbConn,
    GpDbQueryPrepared::CSP  aQueryPrepared,
    const count_t           aMinResultRowsCount,
    GpTaskFiberBarrier::SP  aTaskBarrier
):
GpSocketTask
(
    aName,
    aDbConn->EventPoller(),
    GpSocketTCP::SFromID(aDbConn->SocketId(), GpSocket::CloseModeT::NO_CLOSE_ON_DESTRUCT, GpSocketTCP::StateT::INCOMING)
),
iDbConn(std::move(aDbConn)),
iQueryPrepared(std::move(aQueryPrepared)),
iMinResultRowsCount(aMinResultRowsCount),
iTaskBarrier(std::move(aTaskBarrier))
{
}

GpDbQueryAsyncTask::~GpDbQueryAsyncTask (void) noexcept
{
    if (iTaskBarrier.IsNotNULL())
    {
        iTaskBarrier->Release(std::nullopt);
        iTaskBarrier.Clear();
    }
}

GpTask::ResT    GpDbQueryAsyncTask::OnSockReadyToRead (GpSocket& /*aSocket*/)
{
    try
    {
        return ProcessR();
    } catch (...)
    {
        if (iTaskBarrier.IsNotNULL())
        {
            iTaskBarrier->Release(std::nullopt);
            iTaskBarrier.Clear();
        }

        throw;
    }
}

GpTask::ResT    GpDbQueryAsyncTask::OnSockReadyToWrite (GpSocket& /*aSocket*/)
{
    try
    {
        return ProcessW();
    } catch (...)
    {
        if (iTaskBarrier.IsNotNULL())
        {
            iTaskBarrier->Release(std::nullopt);
            iTaskBarrier.Clear();
        }

        throw;
    }
}

void    GpDbQueryAsyncTask::OnSockClosed (GpSocket& /*aSocket*/)
{
    if (iTaskBarrier.IsNotNULL())
    {
        iTaskBarrier->Release(std::nullopt);
        iTaskBarrier.Clear();
    }
}

void    GpDbQueryAsyncTask::OnSockError (GpSocket& /*aSocket*/)
{
    if (iTaskBarrier.IsNotNULL())
    {
        iTaskBarrier->Release(std::nullopt);
        iTaskBarrier.Clear();
    }
}

GpTask::ResT    GpDbQueryAsyncTask::ProcessR (void)
{
    PGconn* pgConn = iDbConn->PgConn();

    const int rc = PQconsumeInput(pgConn);

    if (rc == 0)
    {
        if (iTaskBarrier.IsNotNULL())
        {
            iTaskBarrier->Release(std::nullopt);
            iTaskBarrier.Clear();
        }

        return GpTask::ResT::DONE;
    }

    PGnotify* pgNotify = nullptr;

    while ((pgNotify = PQnotifies(pgConn)) != nullptr)
    {
        PQfreemem(pgNotify);
    }

    if (PQisBusy(pgConn) == 0)
    {
        PGresult* pgResult = nullptr;

        while ((pgResult = PQgetResult(pgConn)) != nullptr)
        {
            if (iTaskBarrier.IsNotNULL())
            {
                iTaskBarrier->Release(MakeSP<GpDbQueryResPgSql>(pgResult));
                iTaskBarrier.Clear();
            }
        }

        return GpTask::ResT::DONE;
    }

    return GpTask::ResT::WAITING;
}

GpTask::ResT    GpDbQueryAsyncTask::ProcessW (void)
{
    if (iIsSend == false)
    {
        Send();
        iIsSend = true;
    }

    return GpTask::ResT::WAITING;
}

void    GpDbQueryAsyncTask::Send (void)
{
    const GpDbQueryPreparedPgSql&   queryPrepared   = static_cast<const GpDbQueryPreparedPgSql&>(iQueryPrepared.VC());
    const GpDbQuery&                query           = queryPrepared.Query().VC();
    std::string_view                queryStr        = query.QueryStr();
    std::string                     queryZT;

    queryZT.reserve(NumOps::SAdd<size_t>(queryStr.length(), 1));
    queryZT.append(queryStr).append("\0"_sv);

    const int sendRes = PQsendQueryParams
    (
        iDbConn->PgConn(),
        queryZT.data(),
        int(query.Values().size()),
        queryPrepared.OIDs().data(),
        queryPrepared.ValuesPtr().data(),
        queryPrepared.ValuesSize().data(),
        queryPrepared.ValuesIsBinary().data(),
        int(GpPosrgresQueryResultType::BINARY)
    );

    THROW_DBE_COND
    (
        sendRes == 1,
        GpDbExceptionCode::QUERY_ERROR,
        [&](){return "Failed to do SQL query: "_sv + std::string_view(PQerrorMessage(iDbConn->PgConn()));}
    );
}

}//namespace GPlatform

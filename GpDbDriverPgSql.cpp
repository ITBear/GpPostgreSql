#include "GpDbDriverPgSql.hpp"
#include "GpDbConnectionPgSql.hpp"

namespace GPlatform {

GpDbDriverPgSql::GpDbDriverPgSql (void) noexcept:
GpDbDriver("postgresql"_sv)
{
}

GpDbDriverPgSql::~GpDbDriverPgSql (void) noexcept
{
}

GpDbConnection::SP  GpDbDriverPgSql::NewConnection
(
    const GpDbConnectionMode::EnumT aMode,
    std::string_view                aConnStr
) const
{
    GpDbConnection::SP connection;

    switch (aMode)
    {
        case GpDbConnectionMode::SYNC:
        {
            connection = MakeSP<GpDbConnectionPgSql>(ConnectSync(aConnStr), aMode);
        } break;
        case GpDbConnectionMode::ASYNC:
        {
            connection = MakeSP<GpDbConnectionPgSql>(ConnectAsync(aConnStr), aMode);
        } break;
        default:
        {
            THROW_GPE("Unknown connection mode"_sv);
        }
    }

    return connection;
}

PGconn* GpDbDriverPgSql::ConnectSync (std::string_view aConnStr) const
{
    PGconn* pgConn = PQconnectdb(aConnStr.data());

    THROW_GPE_COND(pgConn != nullptr, "PQconnectdb return null"_sv);

    if (PQstatus(pgConn) == CONNECTION_BAD)
    {
        const std::string errMsg = PQerrorMessage(pgConn);
        PQfinish(pgConn);
        THROW_GPE("PQconnectdb return: "_sv + errMsg);
    }

    return pgConn;
}

PGconn* GpDbDriverPgSql::ConnectAsync (std::string_view /*aConnStr*/) const
{
    THROW_GPE_COND
    (
        GpTaskFiber::SIsIntoFiber(),
        "Async connection available only from inside fiber task"_sv
    );

    THROW_GPE_NOT_IMPLEMENTED();

    /*try
    {
        if (Status() != StatusTE::CLOSED)
        {
            THROW_GP_EXCEPTION("Failed to open connection, current status must be CLOSED, but current value "_sv
                               + GpDbConnectionStatus::SToString(Status()));
        }

        SetStatus(StatusTE::CONNECTION_IN_PROGRESS);

        //Begin connection
        PGconn* pgConn = PQconnectStart(aConnStr.data());
        iPgConn = pgConn;

        if (pgConn == nullptr)
        {
            THROW_GP_EXCEPTION("Failed to allocate memory for PGconn"_sv);
        }

        //Check status
        if (PQstatus(pgConn) == CONNECTION_BAD)
        {
            CloseAndThrow("Failed to connect. "_sv);
        }

        PQsetnonblocking(pgConn, 1);

        //Create IO device watcher and add to epoll
        iConnIoWatcher = GpIODeviceWatcher::SP::SNew(PQsocket(pgConn));
        GpApp::S().IONotificatorTask().Vn().AddDeviceWatcher(iConnIoWatcher);//add to epoll

        //
        GpIODeviceWatcherTaskNotifyGuard ioNotyfyGuard(iConnIoWatcher, GpTaskCoroutineCtx::SCurrentTask());

        do
        {
            //Check
            PostgresPollingStatusType pgPoolRes = PQconnectPoll(pgConn);

            if (pgPoolRes == PGRES_POLLING_FAILED)
            {
                CloseAndThrow("Failed to connect. "_sv);
            } else if (pgPoolRes == PGRES_POLLING_OK)
            {
                SetStatus(StatusTE::CONNECTED);
                return;
            } else
            {
                PQconnectPoll(pgConn);
            }

            //Return to waiting
            GpTaskCoroutineCtx::SYield(GpTaskExecRes::WAITING);
        } while(true);
    } catch (...)
    {
        Close();
        throw;
    }*/
}

}//namespace GPlatform

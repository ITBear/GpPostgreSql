#include "GpDbDriverPgSql.hpp"
#include "GpDbConnectionPgSql.hpp"
#include "GpDbConnectAsyncTask.hpp"
#include "GpDbQueryPreparedPgSql.hpp"

namespace GPlatform {

GpDbDriverPgSql::GpDbDriverPgSql
(
    const GpDbConnectionMode::EnumT aMode,
    GpIOEventPoller::WP             aEventPoller
):
GpDbDriver("postgresql"_sv, aMode, aEventPoller)
{
}

GpDbDriverPgSql::~GpDbDriverPgSql (void) noexcept
{
}

GpDbConnection::SP  GpDbDriverPgSql::NewConnection (std::string_view aConnStr) const
{
    GpDbConnection::SP connection;

    switch (Mode())
    {
        case GpDbConnectionMode::SYNC:
        {
            connection = MakeSP<GpDbConnectionPgSql>(ConnectSync(aConnStr), Mode(), GpIOEventPoller::SP::SNull());
            connection.Vn().SetSelfWP(connection);
        } break;
        case GpDbConnectionMode::ASYNC:
        {
            connection = MakeSP<GpDbConnectionPgSql>(ConnectAsync(aConnStr), Mode(), EventPoller());
            connection.Vn().SetSelfWP(connection);
        } break;
        default:
        {
            THROW_GPE("Unknown connection mode"_sv);
        }
    }

    return connection;
}

GpDbQueryPrepared::CSP  GpDbDriverPgSql::Prepare (GpDbQuery::CSP aQuery) const
{
    GpDbQueryPreparedPgSql::SP queryPrepared = MakeSP<GpDbQueryPreparedPgSql>(aQuery);
    queryPrepared->Prepare();

    return queryPrepared;
}

PGconn* GpDbDriverPgSql::ConnectSync (std::string_view aConnStr) const
{
    const std::string connStr(aConnStr);
    PGconn* pgConn = PQconnectdb(connStr.data());

    THROW_GPE_COND(pgConn != nullptr, "PQconnectdb return null"_sv);

    if (PQstatus(pgConn) == CONNECTION_BAD)
    {
        const std::string errMsg = PQerrorMessage(pgConn);
        PQfinish(pgConn);
        THROW_GPE("PQconnectdb return: "_sv + errMsg);
    }

    return pgConn;
}

PGconn* GpDbDriverPgSql::ConnectAsync (std::string_view aConnStr) const
{
    //https://gist.github.com/ictlyh/6a09e8b3847199c15986d476478072e0

    THROW_GPE_COND
    (
        GpTaskFiber::SIsIntoFiber(),
        "Async connection available only from inside fiber task"_sv
    );

    const std::string connStr(aConnStr);

    //Allocate
    PGconn* pgConn = PQconnectStart(connStr.data());

    GpOnThrowStackUnwindFn<std::function<void()>> onThrowStackUnwind;
    onThrowStackUnwind.Push([&](){if (pgConn != nullptr) {PQfinish(pgConn);}});

    THROW_DBE_COND
    (
        pgConn != nullptr,
        GpDbExceptionCode::CONNECTION_ERROR,
        "Failed to allocate memory for PGconn"_sv
    );

    //Check status
    THROW_DBE_COND
    (
        PQstatus(pgConn) != CONNECTION_BAD,
        GpDbExceptionCode::CONNECTION_ERROR,
        [&](){return "Failed to connect to DB: "_sv + std::string_view(PQerrorMessage(pgConn));}
    );

    //Set non blocking
    PQsetnonblocking(pgConn, 1);

    //Create connection task and wait
    GpTaskFiberBarrier::SP      taskBarrier = MakeSP<GpTaskFiberBarrier>(1_cnt);
    GpDbConnectAsyncTask::SP    task        = MakeSP<GpDbConnectAsyncTask>
    (
        Name() + ": async connection to DB"_sv,
        EventPoller(),
        pgConn,
        taskBarrier
    );

    GpTaskScheduler::SCurrentScheduler().value()->AddTaskToWaiting(task);
    EventPoller()->AddSubscriber(task, task->Socket().Id());

    const auto& res = taskBarrier->Wait();

    //Check result
    THROW_DBE_COND
    (
           res.at(0).has_value()
        && std::any_cast<GpDbConnectionStatus>(res.at(0).value()) == GpDbConnectionStatus::CONNECTED,
        GpDbExceptionCode::CONNECTION_ERROR,
        [&](){return "Failed to connect to DB: "_sv + std::string_view(PQerrorMessage(pgConn));}
    );

    return pgConn;
}

}//namespace GPlatform

#include "GpDbConnectionPgSql.hpp"
#include "GpDbQueryPreparedPgSql.hpp"
#include "GpDbQueryResPgSql.hpp"
#include "GpDbQueryAsyncTask.hpp"

#include <iostream>

//https://gist.github.com/ictlyh/12fe787ec265b33fd7e4b0bd08bc27cb

namespace GPlatform {

GpDbConnectionPgSql::IsolationLevelNamesT   GpDbConnectionPgSql::sIsolationLevelNames =
{
    "SERIALIZABLE"_sv,
    "REPEATABLE READ"_sv,
    "READ COMMITTED"_sv,
    "READ UNCOMMITTED"_sv
};

GpDbConnectionPgSql::GpDbConnectionPgSql
(
    PGconn*             aPgConn,
    const ModeTE        aMode,
    GpIOEventPoller::WP aEventPoller
) noexcept:
GpDbConnection(StatusTE::CONNECTED, aMode, aEventPoller),
iPgConn(aPgConn)
{
}

GpDbConnectionPgSql::~GpDbConnectionPgSql (void) noexcept
{
    ClosePgConn();
}

void    GpDbConnectionPgSql::Close (void)
{
    ClosePgConn();
}

GpDbQueryRes::SP    GpDbConnectionPgSql::Execute
(
    GpDbQueryPrepared::CSP  aQueryPrepared,
    const count_t           aMinResultRowsCount
)
{
    //TODO: reimplement with logger listener
    std::cout << "[GpDbConnectionPgSql::Execute]: SQL '" << aQueryPrepared.VC().Query().VC().QueryStr() << "'" << std::endl;

    switch (Mode())
    {
        case GpDbConnectionMode::SYNC:
        {
            return ExecuteSync(aQueryPrepared, aMinResultRowsCount);
        } break;
        case GpDbConnectionMode::ASYNC:
        {
            return ExecuteAsync(aQueryPrepared, aMinResultRowsCount);
        } break;
        default:
        {
            THROW_GPE("Unknown connection mode"_sv);
        }
    }
}

std::string GpDbConnectionPgSql::StrEscape (std::string_view aStr) const
{
    PGconn* pgConn = static_cast<PGconn*>(iPgConn);

    if (pgConn == nullptr)
    {
        return std::string();
    }

    char* escapedStrPtr = PQescapeLiteral(pgConn, aStr.data(), aStr.size());
    std::string escapedStr(escapedStrPtr);
    PQfreemem(escapedStrPtr);

    return escapedStr;
}

/*GpDbQueryRes::SP  GpDbConnectionPgSql::Execute
(
    std::string_view    aSQL,
    const count_t       aMinResultRowsCount
)
{
    //TODO: reimplement with logger listener
    std::cout << "[GpDbConnectionPgSql::Execute]: SQL '" << aSQL << "'" << std::endl;

    switch (Mode())
    {
        case GpDbConnectionMode::SYNC:
        {
            return ExecuteSync(aSQL, aMinResultRowsCount);
        } break;
        case GpDbConnectionMode::ASYNC:
        {
            return ExecuteAsync(aSQL, aMinResultRowsCount);
        } break;
        default:
        {
            THROW_GPE("Unknown connection mode"_sv);
        }
    }
}*/

bool    GpDbConnectionPgSql::Validate (void) const noexcept
{
    if (iPgConn == nullptr)
    {
        return false;
    }

    const ConnStatusType connectionStatus = PQstatus(iPgConn);

    return connectionStatus != CONNECTION_BAD;
}

void    GpDbConnectionPgSql::OnBeginTransaction (GpDbTransactionIsolation::EnumT aIsolationLevel)
{
    GpDbQuery::CSP          query           = MakeSP<GpDbQuery>("BEGIN ISOLATION LEVEL "_sv + sIsolationLevelNames.at(size_t(aIsolationLevel)));
    GpDbQueryPrepared::SP   queryPrepared   = MakeSP<GpDbQueryPreparedPgSql>(query);
    queryPrepared.Vn().Prepare();

    Execute(queryPrepared, 0_cnt);
}

void    GpDbConnectionPgSql::OnCommitTransaction (void)
{
    GpDbQuery::CSP          query           = MakeSP<GpDbQuery>("COMMIT"_s);
    GpDbQueryPrepared::SP   queryPrepared   = MakeSP<GpDbQueryPreparedPgSql>(query);
    queryPrepared.Vn().Prepare();

    Execute(queryPrepared, 0_cnt);
}

void    GpDbConnectionPgSql::OnRollbackTransaction (void)
{
    GpDbQuery::CSP          query           = MakeSP<GpDbQuery>("ROLLBACK"_s);
    GpDbQueryPrepared::SP   queryPrepared   = MakeSP<GpDbQueryPreparedPgSql>(query);
    queryPrepared.Vn().Prepare();

    Execute(queryPrepared, 0_cnt);
}

GpDbQueryRes::SP    GpDbConnectionPgSql::ExecuteSync
(
    GpDbQueryPrepared::CSP  aQueryPrepared,
    const count_t           aMinResultRowsCount
)
{
    const GpDbQueryPreparedPgSql&   queryPrepared   = static_cast<const GpDbQueryPreparedPgSql&>(aQueryPrepared.VC());
    const GpDbQuery&                query           = queryPrepared.Query().VC();
    std::string_view                queryStr        = query.QueryStr();
    std::string                     queryZT;

    queryZT.reserve(NumOps::SAdd<size_t>(queryStr.length(), 1));
    queryZT.append(queryStr).append("\0"_sv);

    PGresult* pgResult = PQexecParams
    (
        iPgConn,
        queryZT.data(),
        int(query.Values().size()),
        queryPrepared.OIDs().data(),
        queryPrepared.ValuesPtr().data(),
        queryPrepared.ValuesSize().data(),
        queryPrepared.ValuesIsBinary().data(),
        int(GpPosrgresQueryResultType::BINARY)
    );

    GpDbQueryResPgSql::SP res = MakeSP<GpDbQueryResPgSql>(pgResult);
    res.Vn().Process(aMinResultRowsCount, iPgConn);

    return res;
}

GpDbQueryRes::SP    GpDbConnectionPgSql::ExecuteAsync
(
    GpDbQueryPrepared::CSP  aQueryPrepared,
    const count_t           aMinResultRowsCount
)
{
    //https://gist.github.com/ictlyh/6a09e8b3847199c15986d476478072e0

    //Create SQL requesr task and wait
    GpTaskFiberBarrier::SP  taskBarrier = MakeSP<GpTaskFiberBarrier>(1_cnt);
    GpDbQueryAsyncTask::SP  queryTask   = MakeSP<GpDbQueryAsyncTask>
    (
        "SQL async query"_sv,
        SelfWP().As<GpDbConnectionPgSql::SP>(),
        aQueryPrepared,
        aMinResultRowsCount,
        taskBarrier
    );

    GpTaskScheduler::SCurrentScheduler().value()->AddTaskToWaiting(queryTask);
    EventPoller()->AddSubscriber(queryTask, queryTask->Socket().Id());

    const auto& taskRes = taskBarrier->Wait();

    //Check result
    THROW_DBE_COND
    (
        taskRes.at(0).has_value(),
        GpDbExceptionCode::QUERY_ERROR,
        [&](){return "Failed to do query: "_sv + std::string_view(PQerrorMessage(iPgConn));}
    );

    GpDbQueryResPgSql::SP res = std::any_cast<GpDbQueryResPgSql::SP>(taskRes.at(0).value());
    res->Process(aMinResultRowsCount, iPgConn);

    return res;
}

/*GpDbQueryRes::SP  GpDbConnectionPgSql::ExecuteSync
(
    std::string_view    aSQL,
    const count_t       aMinResultRowsCount
)
{
    std::string_view    queryStr = aSQL;
    std::string         queryZT;

    queryZT.reserve(NumOps::SAdd<size_t>(queryStr.length(), 1));
    queryZT.append(queryStr).append("\0"_sv);

    PGresult* pgResult = PQexecParams
    (
        iPgConn,
        queryZT.data(),
        0,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        int(GpPosrgresQueryResultType::BINARY)
    );

    GpDbQueryResPgSql::SP res = MakeSP<GpDbQueryResPgSql>(pgResult);
    res.Vn().Process(aMinResultRowsCount, iPgConn);

    return res;
}*/

void    GpDbConnectionPgSql::ClosePgConn (void) noexcept
{
    PQfinish(iPgConn);
    iPgConn = nullptr;
    SetStatus(StatusTE::CLOSED);
}

}//namespace GPlatform

#include "GpDbConnectionPgSql.hpp"
#include "GpDbQueryPrepPgSql.hpp"
#include "GpDbQueryResPgSql.hpp"
#include <iostream>

//https://gist.github.com/ictlyh/12fe787ec265b33fd7e4b0bd08bc27cb

namespace GPlatform {

enum class GpPosrgresQueryResultType
{
    TEXT    = 0,
    BINARY  = 1
};

GpDbConnectionPgSql::IsolationLevelNamesT   GpDbConnectionPgSql::sIsolationLevelNames =
{
    "SERIALIZABLE"_sv,
    "REPEATABLE READ"_sv,
    "READ COMMITTED"_sv,
    "READ UNCOMMITTED"_sv
};

GpDbConnectionPgSql::GpDbConnectionPgSql (PGconn*       aPgConn,
                                          const ModeTE  aMode) noexcept:
GpDbConnection(StatusTE::CONNECTED, aMode),
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

GpDbQueryRes::SP    GpDbConnectionPgSql::Execute (const GpDbQuery&  aQuery,
                                                  const count_t     aMinResultRowsCount)
{
    //TODO: reimplement with logger listener
    std::cout << "[GpDbConnectionPgSql::Execute]: SQL '" << aQuery.QueryStr() << "'" << std::endl;

    switch (Mode())
    {
        case GpDbConnectionMode::SYNC:
        {
            return ExecuteSync(aQuery, aMinResultRowsCount);
        } break;
        case GpDbConnectionMode::ASYNC:
        {
            return ExecuteAsync(aQuery, aMinResultRowsCount);
        } break;
        default:
        {
            THROW_GPE("Unknown connection mode"_sv);
        }
    }
}

GpDbQueryRes::SP    GpDbConnectionPgSql::Execute (std::string_view  aSQL,
                                                  const count_t     aMinResultRowsCount)
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
}

GpDbQuery::SP   GpDbConnectionPgSql::NewQuery (std::string_view aQueryStr) const
{
    return MakeSP<GpDbQuery>(aQueryStr);
}

GpDbQuery::SP   GpDbConnectionPgSql::NewQuery (std::string_view                     aQueryStr,
                                               const GpDbQuery::ValuesTypesVecT&    aValuesTypes) const
{
    return MakeSP<GpDbQuery>(aQueryStr, aValuesTypes);
}

void    GpDbConnectionPgSql::OnBeginTransaction (GpDbTransactionIsolation::EnumT aIsolationLevel)
{
    Execute("BEGIN ISOLATION LEVEL "_sv + sIsolationLevelNames.at(size_t(aIsolationLevel)), 0_cnt);
}

void    GpDbConnectionPgSql::OnCommitTransaction (void)
{
    Execute("COMMIT"_s, 0_cnt);
}

void    GpDbConnectionPgSql::OnRollbackTransaction (void)
{
    Execute("ROLLBACK"_sv, 0_cnt);
}

GpDbQueryRes::SP    GpDbConnectionPgSql::ExecuteSync (const GpDbQuery&  aQuery,
                                                      const count_t     aMinResultRowsCount)
{
    std::string_view    queryStr = aQuery.QueryStr();
    std::string         queryZT;

    queryZT.reserve(NumOps::SAdd<size_t>(queryStr.length(), 1));
    queryZT.append(queryStr).append("\0"_sv);

    GpDbQueryPrepPgSql queryPrep;
    queryPrep.Prepare(aQuery);

    PGresult* pgResult = PQexecParams(iPgConn,
                                      queryZT.data(),
                                      int(aQuery.Values().size()),
                                      nullptr,
                                      queryPrep.ValuesPtr().data(),
                                      queryPrep.ValuesSize().data(),
                                      queryPrep.ValuesIsBinary().data(),
                                      int(GpPosrgresQueryResultType::BINARY));

    return ProcessResult(pgResult, aMinResultRowsCount);
}

GpDbQueryRes::SP    GpDbConnectionPgSql::ExecuteAsync (const GpDbQuery& /*aQuery*/,
                                                       const count_t    /*aMinResultRowsCount*/)
{
    //https://gist.github.com/ictlyh/6a09e8b3847199c15986d476478072e0
    THROW_GPE_COND_CHECK_M(GpTaskFiberCtx::SIsIntoFiber(), "Async exec available only from inside fiber task"_sv);
    THROW_NOT_IMPLEMENTED();
}

GpDbQueryRes::SP    GpDbConnectionPgSql::ExecuteSync (std::string_view  aSQL,
                                                      const count_t     aMinResultRowsCount)
{
    std::string_view    queryStr = aSQL;
    std::string         queryZT;

    queryZT.reserve(NumOps::SAdd<size_t>(queryStr.length(), 1));
    queryZT.append(queryStr).append("\0"_sv);

    PGresult* pgResult = PQexecParams(iPgConn,
                                      queryZT.data(),
                                      0,
                                      nullptr,
                                      nullptr,
                                      nullptr,
                                      nullptr,
                                      int(GpPosrgresQueryResultType::BINARY));

    return ProcessResult(pgResult,  aMinResultRowsCount);
}

GpDbQueryRes::SP    GpDbConnectionPgSql::ExecuteAsync (std::string_view /*aSQL*/,
                                                       const count_t    /*aMinResultRowsCount*/)
{
    THROW_GPE_COND_CHECK_M(GpTaskFiberCtx::SIsIntoFiber(), "Async exec available only from inside fiber task"_sv);
    THROW_NOT_IMPLEMENTED();
}

GpDbQueryRes::SP    GpDbConnectionPgSql::ProcessResult (PGresult*       aPgResult,
                                                        const count_t   aMinResultRowsCount)
{
    THROW_GPE_COND_CHECK_M(aPgResult != nullptr, "PGresult is null: "_sv + std::string_view(PQerrorMessage(iPgConn)));

    const ExecStatusType    pgResStatus = PQresultStatus(aPgResult);
    GpDbQueryResPgSql::SP   res;
    std::string_view        errMsg;

    switch (pgResStatus)
    {
        case PGRES_EMPTY_QUERY:     // empty query string was executed
        {
            if (aMinResultRowsCount > 0_cnt)
            {
                THROW_DBE(GpDbExceptionCode::EMPTY_QUERY_RES);
            } else
            {
                res = MakeSP<GpDbQueryResPgSql>(aPgResult);
            }
        } break;
        case PGRES_COMMAND_OK:      // a query command that doesn't return anything was executed properly by the backend
        case PGRES_TUPLES_OK:       // a query command that returns tuples was executed properly by the backend, PGresult contains the result tuples
        case PGRES_SINGLE_TUPLE:    // single tuple from larger resultset
        case PGRES_NONFATAL_ERROR:  // notice or warning message
        {
            res = MakeSP<GpDbQueryResPgSql>(aPgResult);
        } break;
        case PGRES_COPY_OUT:        // copy Out data transfer in progress
        {
            errMsg = "copy Out data transfer in progress"_sv;
        } break;
        case PGRES_COPY_IN:         // copy In data transfer in progress
        {
            errMsg = "copy In data transfer in progress"_sv;
        } break;
        case PGRES_BAD_RESPONSE:    // an unexpected response was recv'd from the backend
        {
            errMsg = "an unexpected response was recv'd from the backend"_sv;
        } break;
        case PGRES_FATAL_ERROR:     // query failed
        {
            errMsg = "query failed"_sv;
        } break;
        case PGRES_COPY_BOTH:       // Copy In/Out data transfer in progress
        {
            errMsg = "Copy In/Out data transfer in progress"_sv;
        } break;
        default:
        {
            errMsg = "Unknown error"_sv;
        }
    }

    if (res.IsNULL())
    {
        PQclear(aPgResult);
        THROW_GPE(errMsg + ": "_sv + std::string_view(PQerrorMessage(iPgConn)));
    }

    if (aMinResultRowsCount > 0_cnt)
    {
        if (res.VCn().RowsCount() < aMinResultRowsCount)
        {
            THROW_DBE(GpDbExceptionCode::EMPTY_QUERY_RES);
        }
    }

    return res;
}

void    GpDbConnectionPgSql::ClosePgConn (void) noexcept
{
    if (Mode() == ModeTE::ASYNC)
    {
        std::cout << "[GpDbConnectionPgSql::ClosePgConn]: Mode() == ModeTE::ASYNC. THROW_NOT_IMPLEMENTED..." << std::endl;
        //TODO: implement
        THROW_NOT_IMPLEMENTED();
    }

    PQfinish(iPgConn);
    iPgConn = nullptr;
    SetStatus(StatusTE::CLOSED);
}

}//namespace GPlatform

#pragma once

#include "GpPostgreSql_global.hpp"
#include "GpDbQueryPreparedPgSql.hpp"
#include <postgresql/libpq-fe.h>

namespace GPlatform {

enum class GpPosrgresQueryResultType
{
    TEXT    = 0,
    BINARY  = 1
};

class GpDbConnectionPgSql final: public GpDbConnection      
{
public:
    CLASS_REMOVE_CTRS_DEFAULT_MOVE_COPY(GpDbConnectionPgSql)
    CLASS_DECLARE_DEFAULTS(GpDbConnectionPgSql)

    using IsolationLevelNamesT = std::array<std::string_view, GpDbTransactionIsolation::SCount().Value()>;

public:
                                GpDbConnectionPgSql     (PGconn*                aPgConn,
                                                         const ModeTE           aMode,
                                                         GpIOEventPoller::WP    aEventPoller) noexcept;
    virtual                     ~GpDbConnectionPgSql    (void) noexcept override final;

    GpSocketAddr::SocketIdT     SocketId                (void) const {return PQsocket(iPgConn);}
    PGconn*                     PgConn                  (void) {return iPgConn;}

    virtual void                Close                   (void) override final;
    virtual GpDbQueryRes::SP    Execute                 (GpDbQueryPrepared::CSP aQueryPrepared,
                                                         const count_t          aMinResultRowsCount) override final;
    virtual bool                Validate                (void) const noexcept override final;

protected:
    virtual void                OnBeginTransaction      (GpDbTransactionIsolation::EnumT aIsolationLevel) override final;
    virtual void                OnCommitTransaction     (void) override final;
    virtual void                OnRollbackTransaction   (void) override final;

private:
    GpDbQueryRes::SP            ExecuteSync             (GpDbQueryPrepared::CSP aQueryPrepared,
                                                         const count_t          aMinResultRowsCount);
    GpDbQueryRes::SP            ExecuteAsync            (GpDbQueryPrepared::CSP aQueryPrepared,
                                                         const count_t          aMinResultRowsCount);
    /*GpDbQueryRes::SP          ExecuteSync             (std::string_view       aSQL,
                                                         const count_t          aMinResultRowsCount);
    GpDbQueryRes::SP            ExecuteAsync            (std::string_view       aSQL,
                                                         const count_t          aMinResultRowsCount);*/

    void                        ClosePgConn             (void) noexcept;

private:
    PGconn*                     iPgConn = nullptr;
    static IsolationLevelNamesT sIsolationLevelNames;
};

}//namespace GPlatform

#pragma once

#include "GpPostgreSql_global.hpp"
#include <postgresql/libpq-fe.h>

namespace GPlatform {

class GpDbDriverPgSql final: public GpDbDriver
{
public:
    CLASS_REMOVE_CTRS_MOVE_COPY(GpDbDriverPgSql)
    CLASS_DECLARE_DEFAULTS(GpDbDriverPgSql)
    CLASS_TAG(THREAD_SAFE)

public:
                                    GpDbDriverPgSql     (const GpDbConnectionMode::EnumT    aMode,
                                                         GpIOEventPoller::WP                aEventPoller);
    virtual                         ~GpDbDriverPgSql    (void) noexcept override final;

    virtual GpDbConnection::SP      NewConnection       (std::string_view aConnStr) const override final;
    virtual GpDbQueryPrepared::CSP  Prepare             (GpDbQuery::CSP aQuery) const override final;

private:
    PGconn*                         ConnectSync         (std::string_view aConnStr) const;
    PGconn*                         ConnectAsync        (std::string_view aConnStr) const;
};

}//namespace GPlatform

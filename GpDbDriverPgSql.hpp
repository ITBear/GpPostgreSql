#pragma once

#include "GpPostgreSql_global.hpp"
#include <postgresql/libpq-fe.h>

namespace GPlatform {

class GpDbDriverPgSql final: public GpDbDriver
{
public:
    CLASS_REMOVE_CTRS_EXCEPT_DEFAULT(GpDbDriverPgSql)
    CLASS_DECLARE_DEFAULTS(GpDbDriverPgSql)

public:
                                    GpDbDriverPgSql     (void) noexcept;
    virtual                         ~GpDbDriverPgSql    (void) noexcept override final;

    virtual GpDbConnection::SP      NewConnection       (const GpDbConnectionMode::EnumT    aMode,
                                                         std::string_view                   aConnStr) const override final;

private:
    PGconn*                         ConnectSync         (std::string_view aConnStr) const;
    PGconn*                         ConnectAsync        (std::string_view aConnStr) const;
};

}//namespace GPlatform

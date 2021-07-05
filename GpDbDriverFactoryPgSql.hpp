#pragma once

#include "GpPostgreSql_global.hpp"

namespace GPlatform {

class GPPOSTGRESQL_API GpDbDriverFactoryPgSql final: public GpDbDriverFactory
{
public:
    CLASS_REMOVE_CTRS_EXCEPT_DEFAULT(GpDbDriverFactoryPgSql)
    CLASS_DECLARE_DEFAULTS(GpDbDriverFactoryPgSql)

public:
                                    GpDbDriverFactoryPgSql  (void);
    virtual                         ~GpDbDriverFactoryPgSql (void) noexcept override final;

    virtual GpDbDriver::SP          NewInstance             (const GpDbConnectionMode::EnumT    aMode,
                                                             GpIOEventPoller::WP                aEventPoller) const override final;
};

}//namespace GPlatform

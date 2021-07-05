#include "GpDbDriverFactoryPgSql.hpp"
#include "GpDbDriverPgSql.hpp"

namespace GPlatform {

GpDbDriverFactoryPgSql::GpDbDriverFactoryPgSql (void):
GpDbDriverFactory("postgresql"_sv)
{
}

GpDbDriverFactoryPgSql::~GpDbDriverFactoryPgSql (void) noexcept
{
}

GpDbDriver::SP  GpDbDriverFactoryPgSql::NewInstance
(
    const GpDbConnectionMode::EnumT aMode,
    GpIOEventPoller::WP             aEventPoller) const
{
    return MakeSP<GpDbDriverPgSql>(aMode, aEventPoller);
}

}//namespace GPlatform

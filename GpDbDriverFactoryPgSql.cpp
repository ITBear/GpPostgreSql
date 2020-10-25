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

GpDbDriver::SP  GpDbDriverFactoryPgSql::NewInstance (void) const
{
    return MakeSP<GpDbDriverPgSql>();
}

}//namespace GPlatform

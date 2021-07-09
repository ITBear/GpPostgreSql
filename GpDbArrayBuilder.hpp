#pragma once

#include "GpPostgreSql_global.hpp"
#include <postgresql/libpq-fe.h>

namespace GPlatform {

//https://stackoverflow.com/questions/26499266/whats-the-proper-index-for-querying-structures-in-arrays-in-postgres-jsonb/27708358#27708358

class GpDbArrayBuilder
{
public:
    CLASS_REMOVE_CTRS_DEFAULT_MOVE_COPY(GpDbArrayBuilder)

public:
    static std::tuple<Oid, GpBytesArray>    SBuild  (const GpVector<std::string>& aArray);
};

}//namespace GPlatform

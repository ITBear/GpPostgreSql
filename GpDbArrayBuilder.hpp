#pragma once

#include "GpPostgreSql_global.hpp"
#include <postgresql/libpq-fe.h>

namespace GPlatform {

class GpDbArrayBuilder
{
public:
    CLASS_REMOVE_CTRS(GpDbArrayBuilder)

public:
    static std::tuple<Oid, GpBytesArray>    SBuild  (const GpVector<std::string>& aArray);
};

}//namespace GPlatform

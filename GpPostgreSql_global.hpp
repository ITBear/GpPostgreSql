#pragma once

#include "../GpCore2/GpCore.hpp"
#include "../GpDbClient/GpDbClient.hpp"
#include "../GpLog/GpLog.hpp"

#if defined(GPPOSTGRESQL_LIBRARY)
    #define GPPOSTGRESQL_API GP_DECL_EXPORT
#else
    #define GPPOSTGRESQL_API GP_DECL_IMPORT
#endif

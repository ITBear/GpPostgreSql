#pragma once

#include "GpPostgreSql_global.hpp"
#include <postgresql/libpq-fe.h>

namespace GPlatform {

class GpDbQueryPrepPgSql
{
public:
    CLASS_REMOVE_CTRS_EXCEPT_DEFAULT(GpDbQueryPrepPgSql)
    CLASS_DECLARE_DEFAULTS(GpDbQueryPrepPgSql)

    using ValuesPtrT        = GpVector<const char*>;
    using ValuesSizeT       = GpVector<int>;
    using ValuesIsBinaryT   = GpVector<int>;
    using SInt64VecT        = GpVector<s_int_64>;
    using JsonbDataVecT     = GpVector<GpBytesArray>;

public:
                                GpDbQueryPrepPgSql      (void) noexcept;
                                ~GpDbQueryPrepPgSql     (void) noexcept;

    void                        Prepare                 (const GpDbQuery& aQuery);

    const ValuesPtrT&           ValuesPtr               (void) const noexcept {return iValuesPtr;}
    const ValuesSizeT&          ValuesSize              (void) const noexcept {return iValuesSize;}
    const ValuesIsBinaryT&      ValuesIsBinary          (void) const noexcept {return iValuesIsBinary;}

private:
    void                        FillData                (const GpDbQueryValType::EnumT  aValueType,
                                                         const count_t                  aValueId,
                                                         const GpDbQuery&               aQuery);

private:
    ValuesPtrT                  iValuesPtr;
    ValuesSizeT                 iValuesSize;
    ValuesIsBinaryT             iValuesIsBinary;
    SInt64VecT                  iSInt64Vec;
    JsonbDataVecT               iJsonbDataVec;
};

}//namespace GPlatform

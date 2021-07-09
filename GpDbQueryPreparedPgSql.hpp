#pragma once

#include "GpPostgreSql_global.hpp"
#include <postgresql/libpq-fe.h>

namespace GPlatform {

class GpDbQueryPreparedPgSql final: public GpDbQueryPrepared
{
public:
    CLASS_REMOVE_CTRS_DEFAULT_MOVE_COPY(GpDbQueryPreparedPgSql)
    CLASS_DECLARE_DEFAULTS(GpDbQueryPreparedPgSql)

    using OIDsPtrT          = GpVector<Oid>;
    using ValuesPtrT        = GpVector<const char*>;
    using ValuesSizeT       = GpVector<int>;
    using ValuesIsBinaryT   = GpVector<int>;
    using SInt64VecT        = GpVector<s_int_64>;
    using BinaryDataVecT    = GpVector<GpBytesArray>;

public:
                                GpDbQueryPreparedPgSql  (GpDbQuery::CSP aQuery) noexcept;
    virtual                     ~GpDbQueryPreparedPgSql (void) noexcept override final;

    virtual void                Prepare                 (void) override final;

    const OIDsPtrT&             OIDs                    (void) const noexcept {return iOIDs;}
    const ValuesPtrT&           ValuesPtr               (void) const noexcept {return iValuesPtr;}
    const ValuesSizeT&          ValuesSize              (void) const noexcept {return iValuesSize;}
    const ValuesIsBinaryT&      ValuesIsBinary          (void) const noexcept {return iValuesIsBinary;}

private:
    void                        FillData                (const GpDbQueryValType::EnumT  aValueType,
                                                         const count_t                  aValueId,
                                                         const GpDbQuery&               aQuery);



private:
    OIDsPtrT                    iOIDs;
    ValuesPtrT                  iValuesPtr;
    ValuesSizeT                 iValuesSize;
    ValuesIsBinaryT             iValuesIsBinary;
    SInt64VecT                  iSInt64Vec;
    BinaryDataVecT              iBinaryDataVec;
};

}//namespace GPlatform

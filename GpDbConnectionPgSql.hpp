#pragma once

#include "GpPostgreSql_global.hpp"
#include <postgresql/libpq-fe.h>

namespace GPlatform {

class GpDbConnectionPgSql final: public GpDbConnection
{
public:
    CLASS_REMOVE_CTRS(GpDbConnectionPgSql)
    CLASS_DECLARE_DEFAULTS(GpDbConnectionPgSql)

    using IsolationLevelNamesT = std::array<std::string_view, GpDbTransactionIsolation::SCount().Value()>;

public:
                                GpDbConnectionPgSql     (PGconn*        aPgConn,
                                                         const ModeTE   aMode) noexcept;
    virtual                     ~GpDbConnectionPgSql    (void) noexcept override final;

    virtual void                Close                   (void) override final;
    virtual GpDbQueryRes::SP    Execute                 (const GpDbQuery&   aQuery,
                                                         const count_t      aMinResultRowsCount) override final;
    virtual GpDbQueryRes::SP    Execute                 (std::string_view   aSQL,
                                                         const count_t      aMinResultRowsCount) override final;
    virtual GpDbQuery::SP       NewQuery                (std::string_view aQueryStr) const override final;
    virtual GpDbQuery::SP       NewQuery                (std::string_view                   aQueryStr,
                                                         const GpDbQuery::ValuesTypesVecT&  aValuesTypes) const override final;

protected:
    virtual void                OnBeginTransaction      (GpDbTransactionIsolation::EnumT aIsolationLevel) override final;
    virtual void                OnCommitTransaction     (void) override final;
    virtual void                OnRollbackTransaction   (void) override final;

private:
    GpDbQueryRes::SP            ExecuteSync             (const GpDbQuery&   aQuery,
                                                         const count_t      aMinResultRowsCount);
    GpDbQueryRes::SP            ExecuteAsync            (const GpDbQuery&   aQuery,
                                                         const count_t      aMinResultRowsCount);
    GpDbQueryRes::SP            ExecuteSync             (std::string_view   aSQL,
                                                         const count_t      aMinResultRowsCount);
    GpDbQueryRes::SP            ExecuteAsync            (std::string_view   aSQL,
                                                         const count_t      aMinResultRowsCount);

    GpDbQueryRes::SP            ProcessResult           (PGresult*          aPgResult,
                                                         const count_t      aMinResultRowsCount);

    void                        ClosePgConn             (void) noexcept;

private:
    PGconn*                     iPgConn = nullptr;
    static IsolationLevelNamesT sIsolationLevelNames;

};

}//namespace GPlatform

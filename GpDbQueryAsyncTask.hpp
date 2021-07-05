#pragma once

#include "GpPostgreSql_global.hpp"
#include "GpDbConnectionPgSql.hpp"
#include <postgresql/libpq-fe.h>
#include "../GpNetwork/GpNetwork.hpp"

namespace GPlatform {

class GpDbQueryAsyncTask final: public GpSocketTask
{
public:
    CLASS_REMOVE_CTRS(GpDbQueryAsyncTask)
    CLASS_DECLARE_DEFAULTS(GpDbQueryAsyncTask)

public:
                            GpDbQueryAsyncTask      (std::string_view           aName,
                                                     GpDbConnectionPgSql::SP    aDbConn,
                                                     GpDbQueryPrepared::CSP     aQueryPrepared,
                                                     const count_t              aMinResultRowsCount,
                                                     GpTaskFiberBarrier::SP     aTaskBarrier);
    virtual                 ~GpDbQueryAsyncTask     (void) noexcept override final;

    virtual GpTask::ResT    OnSockReadyToRead       (GpSocket& aSocket) override final;
    virtual GpTask::ResT    OnSockReadyToWrite      (GpSocket& aSocket) override final;
    virtual void            OnSockClosed            (GpSocket& aSocket) override final;
    virtual void            OnSockError             (GpSocket& aSocket) override final;

private:
    GpTask::ResT            ProcessR                (void);
    GpTask::ResT            ProcessW                (void);
    void                    Send                    (void);

private:
    GpDbConnectionPgSql::SP iDbConn;
    GpDbQueryPrepared::CSP  iQueryPrepared;
    const count_t           iMinResultRowsCount;
    GpTaskFiberBarrier::SP  iTaskBarrier;
    bool                    iIsSend = false;
};

}//namespace GPlatform

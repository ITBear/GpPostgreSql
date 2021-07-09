#pragma once

#include "GpPostgreSql_global.hpp"
#include <postgresql/libpq-fe.h>
#include "../GpNetwork/GpNetwork.hpp"

namespace GPlatform {

class GpDbConnectAsyncTask final: public GpSocketTask
{
public:
    CLASS_REMOVE_CTRS_DEFAULT_MOVE_COPY(GpDbConnectAsyncTask)
    CLASS_DECLARE_DEFAULTS(GpDbConnectAsyncTask)

public:
                            GpDbConnectAsyncTask    (std::string_view       aName,
                                                     GpIOEventPoller::WP    aIOPoller,
                                                     PGconn*                aPGconn,
                                                     GpTaskFiberBarrier::SP aTaskBarrier);
    virtual                 ~GpDbConnectAsyncTask   (void) noexcept override final;

    virtual GpTask::ResT    OnSockReadyToRead       (GpSocket& aSocket) override final;
    virtual GpTask::ResT    OnSockReadyToWrite      (GpSocket& aSocket) override final;
    virtual void            OnSockClosed            (GpSocket& aSocket) override final;
    virtual void            OnSockError             (GpSocket& aSocket) override final;

private:
    GpTask::ResT            ProcessRW               (void);

private:
    PGconn*                 iPGconn = nullptr;
    GpTaskFiberBarrier::SP  iTaskBarrier;
};

}//namespace GPlatform

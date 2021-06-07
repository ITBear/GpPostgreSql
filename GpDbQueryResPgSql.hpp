#pragma once

#include "GpPostgreSql_global.hpp"
#include <postgresql/libpq-fe.h>

namespace GPlatform {

class GpDbQueryResPgSql final: public GpDbQueryRes
{
public:
    CLASS_REMOVE_CTRS(GpDbQueryResPgSql)
    CLASS_DECLARE_DEFAULTS(GpDbQueryResPgSql)

public:
                                GpDbQueryResPgSql   (PGresult* aPgResult);
    virtual                     ~GpDbQueryResPgSql  (void) noexcept override final;

    virtual void                Clear               (void) override final;

    [[nodiscard]]
    virtual StateTE             State               (void) const override final;

    [[nodiscard]]
    virtual count_t             RowsCount           (void) const override final;

    [[nodiscard]]
    virtual count_t             ColumnsCount        (void) const override final;

    [[nodiscard]]
    virtual SInt16              GetInt16            (const count_t          aRowId,
                                                     const count_t          aColId,
                                                     std::optional<SInt16>  aOnNullValue) const override final;

    [[nodiscard]]
    virtual SInt32              GetInt32            (const count_t          aRowId,
                                                     const count_t          aColId,
                                                     std::optional<SInt32>  aOnNullValue) const override final;

    [[nodiscard]]
    virtual SInt64              GetInt64            (const count_t          aRowId,
                                                     const count_t          aColId,
                                                     std::optional<SInt64>  aOnNullValue) const override final;

    [[nodiscard]]
    virtual std::string_view    GetStr              (const count_t                      aRowId,
                                                     const count_t                      aColId,
                                                     std::optional<std::string_view>    aOnNullValue) const override final;

    [[nodiscard]]
    virtual GpRawPtrCharRW      GetStrRW            (const count_t                  aRowId,
                                                     const count_t                  aColId,
                                                     std::optional<GpRawPtrCharRW>  aOnNullValue) override final;

    [[nodiscard]]
    virtual const GpVector<std::string>&
                                GetStrArray         (const count_t                      aRowId,
                                                     const count_t                      aColId,
                                                     std::optional<std::string_view>    aOnNullValue) const override final;

    [[nodiscard]]
    virtual std::string_view    GetJsonStr          (const count_t                      aRowId,
                                                     const count_t                      aColId,
                                                     std::optional<std::string_view>    aOnNullValue) const override final;

    [[nodiscard]]
    virtual GpRawPtrCharRW      GetJsonStrRW        (const count_t                  aRowId,
                                                     const count_t                  aColId,
                                                     std::optional<GpRawPtrCharRW>  aOnNullValue) override final;

    [[nodiscard]]
    virtual GpUUID              GetUUID             (const count_t          aRowId,
                                                     const count_t          aColId,
                                                     std::optional<GpUUID>  aOnNullValue) const override final;

    [[nodiscard]]
    virtual GpRawPtrByteR       GetBLOB             (const count_t                  aRowId,
                                                     const count_t                  aColId,
                                                     std::optional<GpRawPtrByteR>   aOnNullValue) const override final;

    [[nodiscard]]
    virtual bool                GetBoolean          (const count_t          aRowId,
                                                     const count_t          aColId,
                                                     std::optional<bool>    aOnNullValue) const override final;

private:
    void                        ClearPgSql          (void) noexcept;

private:
    PGresult*                   iPgResult = nullptr;
};

}//namespace GPlatform

#include "GpDbQueryResPgSql.hpp"

namespace GPlatform {

GpDbQueryResPgSql::GpDbQueryResPgSql (PGresult* aPgResult):
iPgResult(aPgResult)
{
    static_assert(PGRES_EMPTY_QUERY     == 0, "PGRES_EMPTY_QUERY    != 0");
    static_assert(PGRES_COMMAND_OK      == 1, "PGRES_COMMAND_OK     != 1");
    static_assert(PGRES_TUPLES_OK       == 2, "PGRES_TUPLES_OK      != 2");
    static_assert(PGRES_COPY_OUT        == 3, "PGRES_COPY_OUT       != 3");
    static_assert(PGRES_COPY_IN         == 4, "PGRES_COPY_IN        != 4");
    static_assert(PGRES_BAD_RESPONSE    == 5, "PGRES_BAD_RESPONSE   != 5");
    static_assert(PGRES_NONFATAL_ERROR  == 6, "PGRES_NONFATAL_ERROR != 6");
    static_assert(PGRES_FATAL_ERROR     == 7, "PGRES_FATAL_ERROR    != 7");
    static_assert(PGRES_COPY_BOTH       == 8, "PGRES_COPY_BOTH      != 8");
    static_assert(PGRES_SINGLE_TUPLE    == 9, "PGRES_SINGLE_TUPLE   != 9");
}

GpDbQueryResPgSql::~GpDbQueryResPgSql (void) noexcept
{
    ClearPgSql();
}

void    GpDbQueryResPgSql::Process
(
    const count_t   aMinResultRowsCount,
    PGconn*         aPgConn
)
{
    THROW_GPE_COND
    (
        iPgResult != nullptr,
        [&](){return "PGresult is null: "_sv + std::string_view(PQerrorMessage(aPgConn));}
    );

    const ExecStatusType    pgResStatus = PQresultStatus(iPgResult);
    std::string_view        errMsg;

    switch (pgResStatus)
    {
        case PGRES_EMPTY_QUERY:     // empty query string was executed
        {
            if (aMinResultRowsCount > 0_cnt)
            {
                THROW_DBE(GpDbExceptionCode::QUERY_RESULT_COUNT_LOW, ""_sv);
            }
        } break;
        case PGRES_COMMAND_OK:      // a query command that doesn't return anything was executed properly by the backend
        case PGRES_TUPLES_OK:       // a query command that returns tuples was executed properly by the backend, PGresult contains the result tuples
        case PGRES_SINGLE_TUPLE:    // single tuple from larger resultset
        case PGRES_NONFATAL_ERROR:  // notice or warning message
        {
            //OK
        } break;
        case PGRES_COPY_OUT:        // copy Out data transfer in progress
        {
            errMsg = "copy Out data transfer in progress"_sv;
        } break;
        case PGRES_COPY_IN:         // copy In data transfer in progress
        {
            errMsg = "copy In data transfer in progress"_sv;
        } break;
        case PGRES_BAD_RESPONSE:    // an unexpected response was recv'd from the backend
        {
            errMsg = "an unexpected response was recv'd from the backend"_sv;
        } break;
        case PGRES_FATAL_ERROR:     // query failed
        {
            errMsg = "query failed"_sv;
        } break;
        case PGRES_COPY_BOTH:       // Copy In/Out data transfer in progress
        {
            errMsg = "Copy In/Out data transfer in progress"_sv;
        } break;
        default:
        {
            errMsg = "Unknown error"_sv;
        }
    }

    if (errMsg.length() > 0)
    {
        Clear();
        ThrowDbEx(errMsg, aPgConn);
    }

    if (aMinResultRowsCount > 0_cnt)
    {
        if (RowsCount() < aMinResultRowsCount)
        {
            THROW_DBE(GpDbExceptionCode::QUERY_RESULT_COUNT_LOW, "RowsCount() < aMinResultRowsCount"_sv);
        }
    }
}

void    GpDbQueryResPgSql::Clear (void)
{
    ClearPgSql();
}

GpDbQueryRes::StateTE   GpDbQueryResPgSql::State (void) const
{
    const ExecStatusType pgStatus = PQresultStatus(iPgResult);
    return GpDbQueryRes::StateTE(int(pgStatus));
}

count_t GpDbQueryResPgSql::RowsCount (void) const
{
    int rowsCount = PQntuples(iPgResult);

    if (rowsCount < 0)
    {
        rowsCount = 0;
    }

    return count_t::SMake(rowsCount);
}

count_t GpDbQueryResPgSql::ColumnsCount (void) const
{
    int columnsCount= PQnfields(iPgResult);

    if (columnsCount < 0)
    {
        columnsCount = 0;
    }

    return count_t::SMake(columnsCount);
}

s_int_16    GpDbQueryResPgSql::GetInt16
(
    const count_t           aRowId,
    const count_t           aColId,
    std::optional<s_int_16> aOnNullValue
) const
{
    const int rowId = aRowId.As<int>();
    const int colId = aColId.As<int>();

    if (PQgetisnull(iPgResult, rowId, colId))
    {
        THROW_GPE_COND
        (
            aOnNullValue.has_value(),
            [&](){return "Value on ["_sv + aRowId + ", "_sv + aColId + "] is NULL"_sv;}
        );
        return aOnNullValue.value();
    }

    const int len = PQgetlength(iPgResult, rowId, colId);

    THROW_GPE_COND
    (
        size_t(len) == sizeof(s_int_16),
        "s_int_16 length must be 8 bytes"_sv
    );

    s_int_16 value = BitOps::N2H(*reinterpret_cast<const s_int_16*>(PQgetvalue(iPgResult, rowId, colId)));
    return value;
}

s_int_32    GpDbQueryResPgSql::GetInt32
(
    const count_t           aRowId,
    const count_t           aColId,
    std::optional<s_int_32> aOnNullValue
) const
{
    const int rowId = aRowId.As<int>();
    const int colId = aColId.As<int>();

    if (PQgetisnull(iPgResult, rowId, colId))
    {
        THROW_GPE_COND
        (
            aOnNullValue.has_value(),
            [&](){return "Value on ["_sv + aRowId + ", "_sv + aColId + "] is NULL"_sv;}
        );
        return aOnNullValue.value();
    }

    const int len = PQgetlength(iPgResult, rowId, colId);

    THROW_GPE_COND
    (
        size_t(len) == sizeof(s_int_32),
        "s_int_32 length must be 8 bytes"_sv
    );

    s_int_32 value = BitOps::N2H(*reinterpret_cast<const s_int_32*>(PQgetvalue(iPgResult, rowId, colId)));
    return value;
}

s_int_64    GpDbQueryResPgSql::GetInt64
(
    const count_t           aRowId,
    const count_t           aColId,
    std::optional<s_int_64> aOnNullValue
) const
{
    const int rowId = aRowId.As<int>();
    const int colId = aColId.As<int>();

    if (PQgetisnull(iPgResult, rowId, colId))
    {
        THROW_GPE_COND
        (
            aOnNullValue.has_value(),
            [&](){return "Value on ["_sv + aRowId + ", "_sv + aColId + "] is NULL"_sv;}
        );
        return aOnNullValue.value();
    }

    const int len = PQgetlength(iPgResult, rowId, colId);

    THROW_GPE_COND
    (
        size_t(len) == sizeof(s_int_64),
        "s_int_64 length must be 8 bytes"_sv
    );

    s_int_64 value = BitOps::N2H(*reinterpret_cast<const s_int_64*>(PQgetvalue(iPgResult, rowId, colId)));
    return value;
}

std::string_view    GpDbQueryResPgSql::GetStr
(
    const count_t                   aRowId,
    const count_t                   aColId,
    std::optional<std::string_view> aOnNullValue
) const
{
    const int rowId = aRowId.As<int>();
    const int colId = aColId.As<int>();

    if (PQgetisnull(iPgResult, rowId, colId))
    {
        THROW_GPE_COND
        (
            aOnNullValue.has_value(),
            [&](){return "Value on ["_sv + aRowId + ", "_sv + aColId + "] is NULL"_sv;}
        );
        return aOnNullValue.value();
    }

    const char*     strPtr  = PQgetvalue(iPgResult, rowId, colId);
    const size_t    strLen  = NumOps::SConvert<size_t>(PQgetlength(iPgResult, rowId, colId));

    return std::string_view(strPtr, strLen);
}

GpRawPtrCharRW  GpDbQueryResPgSql::GetStrRW
(
    const count_t                   aRowId,
    const count_t                   aColId,
    std::optional<GpRawPtrCharRW>   aOnNullValue
)
{
    std::optional<std::string_view> defaultValue;

    if (aOnNullValue.has_value())
    {
        defaultValue = aOnNullValue.value().AsStringView();
    }

    std::string_view str = std::as_const(*this).GetStr
    (
        aRowId,
        aColId,
        defaultValue
    );

    return GpRawPtrCharRW(const_cast<char*>(str.data()), str.size());
}

const GpVector<std::string>&    GpDbQueryResPgSql::GetStrArray
(
    const count_t                   /*aRowId*/,
    const count_t                   /*aColId*/,
    std::optional<std::string_view> /*aOnNullValue*/
) const
{
    //TODO: implement
    THROW_GPE_NOT_IMPLEMENTED();
}

std::string_view    GpDbQueryResPgSql::GetJsonStr
(
    const count_t                   aRowId,
    const count_t                   aColId,
    std::optional<std::string_view> aOnNullValue
) const
{
    std::string_view str = GetStr(aRowId, aColId, aOnNullValue);

    THROW_GPE_COND
    (
        str.length() >= 3,
        "json data length must be >= 3 bytes"_sv
    );

    THROW_GPE_COND
    (
        std::bit_cast<u_int_8>(str.data()[0]) == 1,
        "Wrong pgJson format version"_sv
    );

    return str.substr(1, str.length() - 1);
}

GpRawPtrCharRW  GpDbQueryResPgSql::GetJsonStrRW
(
    const count_t                   aRowId,
    const count_t                   aColId,
    std::optional<GpRawPtrCharRW>   aOnNullValue
)
{
    std::optional<std::string_view> defaultValue;

    if (aOnNullValue.has_value())
    {
        defaultValue = aOnNullValue.value().AsStringView();
    }

    std::string_view str = std::as_const(*this).GetJsonStr
    (
        aRowId,
        aColId,
        defaultValue
    );

    return GpRawPtrCharRW(const_cast<char*>(str.data()), str.size());
}

GpUUID  GpDbQueryResPgSql::GetUUID
(
    const count_t           aRowId,
    const count_t           aColId,
    std::optional<GpUUID>   aOnNullValue
) const
{
    const int rowId = aRowId.As<int>();
    const int colId = aColId.As<int>();

    if (PQgetisnull(iPgResult, rowId, colId))
    {
        THROW_GPE_COND
        (
            aOnNullValue.has_value(),
            [&](){return "Value on ["_sv + aRowId + ", "_sv + aColId + "] is NULL"_sv;}
        );
        return aOnNullValue.value();
    }

    const char*     strPtr  = PQgetvalue(iPgResult, rowId, colId);
    const size_t    strLen  = NumOps::SConvert<size_t>(PQgetlength(iPgResult, rowId, colId));

    std::string_view str(strPtr, strLen);

    THROW_GPE_COND
    (
        str.length() == sizeof(GpUUID::DataT),
        "uuid length must be 16 bytes"_sv
    );

    GpUUID uuid;
    std::memcpy(uuid.Data().data(),
                str.data(),
                sizeof(GpUUID::DataT));

    return uuid;
}

GpRawPtrByteR   GpDbQueryResPgSql::GetBLOB
(
    const count_t                   aRowId,
    const count_t                   aColId,
    std::optional<GpRawPtrByteR>    aOnNullValue
) const
{
    const int rowId = aRowId.As<int>();
    const int colId = aColId.As<int>();

    if (PQgetisnull(iPgResult, rowId, colId))
    {
        THROW_GPE_COND
        (
            aOnNullValue.has_value(),
            [&](){return "Value on ["_sv + aRowId + ", "_sv + aColId + "] is NULL"_sv;}
        );
        return aOnNullValue.value();
    }

    const char*     strPtr  = PQgetvalue(iPgResult, rowId, colId);
    const size_t    strLen  = NumOps::SConvert<size_t>(PQgetlength(iPgResult, rowId, colId));

    std::string_view str(strPtr, strLen);

    return GpRawPtrCharR(str.data(), str.length());
}

bool    GpDbQueryResPgSql::GetBoolean
(
    const count_t       aRowId,
    const count_t       aColId,
    std::optional<bool> aOnNullValue
) const
{
    const int rowId = aRowId.As<int>();
    const int colId = aColId.As<int>();

    if (PQgetisnull(iPgResult, rowId, colId))
    {
        THROW_GPE_COND
        (
            aOnNullValue.has_value(),
            [&](){return "Value on ["_sv + aRowId + ", "_sv + aColId + "] is NULL"_sv;}
        );
        return aOnNullValue.value();
    }

    const char*     strPtr  = PQgetvalue(iPgResult, rowId, colId);
    const size_t    strLen  = NumOps::SConvert<size_t>(PQgetlength(iPgResult, rowId, colId));

    std::string_view str(strPtr, strLen);
    const char v = str.at(0);

    return     (v == 0x01)
            || (v == 't')
            || (v == 'T')
            || (v == 'y')
            || (v == 'Y');
}

void    GpDbQueryResPgSql::ClearPgSql (void) noexcept
{
    if (iPgResult != nullptr)
    {
        PQclear(iPgResult);
        iPgResult = nullptr;
    }
}

void    GpDbQueryResPgSql::ThrowDbEx
(
    std::string_view    aMsg,
    PGconn*             aPgConn
)
{
    std::string_view            message = std::string_view(PQerrorMessage(aPgConn));
    GpDbExceptionCode::EnumT    code    = GpDbExceptionCode::QUERY_ERROR;

    if (message.find("duplicate key"_sv) != std::string_view::npos)
    {
        code = GpDbExceptionCode::QUERY_DUPLICATE_KEY;
    }

    THROW_DBE
    (
        code,
        aMsg + ": "_sv + message
    );
}

}//namespace GPlatform

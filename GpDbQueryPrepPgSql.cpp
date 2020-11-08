#include "GpDbQueryPrepPgSql.hpp"

namespace GPlatform {

GpDbQueryPrepPgSql::GpDbQueryPrepPgSql (void) noexcept
{
}

GpDbQueryPrepPgSql::~GpDbQueryPrepPgSql (void) noexcept
{
}

void    GpDbQueryPrepPgSql::Prepare (const GpDbQuery& aQuery)
{
    const GpDbQuery::ValuesTypesVecT& valuesTypes = aQuery.ValuesTypes();

    const size_t valuesTypesCount   = valuesTypes.size();
    const size_t valuesCount        = aQuery.Values().size();

    THROW_GPE_COND_CHECK_M(valuesTypesCount == valuesCount, "valuesTypesCount != valuesCount"_sv);

    iValuesPtr.reserve(valuesCount);
    iValuesSize.reserve(valuesCount);
    iValuesIsBinary.reserve(valuesCount);
    iSInt64Vec.reserve(valuesCount);
    iJsonbDataVec.reserve(valuesCount);

    const count_t count = count_t::SMake(valuesCount);
    for (count_t id = 0_cnt; id < count; ++id)
    {
        const GpDbQueryValType::EnumT valueType = valuesTypes.at(id.As<size_t>());
        FillData(valueType, id, aQuery);
    }
}

void    GpDbQueryPrepPgSql::FillData (const GpDbQueryValType::EnumT aValueType,
                                      const count_t                 aValueId,
                                      const GpDbQuery&              aQuery)
{
    switch (aValueType)
    {
        case GpDbQueryValType::INT_64:
        {
            const SInt64 value = aQuery.Int64(aValueId);
            iSInt64Vec.emplace_back(BitOps::N2H(value.As<s_int_64>()));

            iValuesPtr.emplace_back(reinterpret_cast<const char*>(iSInt64Vec.data() + (iSInt64Vec.size() - 1)));
            iValuesSize.emplace_back(NumOps::SConvert<int>(sizeof(s_int_64)));
            iValuesIsBinary.emplace_back(1);
        } break;
        case GpDbQueryValType::STRING_VALUE:
        {
            std::string_view value = aQuery.StrValue(aValueId);

            iValuesPtr.emplace_back(value.data());
            iValuesSize.emplace_back(NumOps::SConvert<int>(value.size()));
            iValuesIsBinary.emplace_back(1);
        } break;
        case GpDbQueryValType::STRING_NAME:
        {
            std::string_view value = aQuery.StrName(aValueId);

            iValuesPtr.emplace_back(value.data());
            iValuesSize.emplace_back(NumOps::SConvert<int>(value.size()));
            iValuesIsBinary.emplace_back(1);
        } break;
        case GpDbQueryValType::STRING_JSON:
        {
            std::string_view value = aQuery.StrJson(aValueId);

            GpBytesArray jsonbData;
            jsonbData.resize(NumOps::SAdd<size_t>(value.size(), 1));
            GpByteWriterStorageFixedSize    writerStorage(jsonbData);
            GpByteWriter                    writer(writerStorage);

            writer.UInt8(1);//Jsonb version
            writer.Bytes(value);

            const std::byte*    dataPtr     = jsonbData.data();
            const size_t        dataSize    = jsonbData.size();

            iJsonbDataVec.emplace_back(std::move(jsonbData));

            //---------------------------
            iValuesPtr.emplace_back(reinterpret_cast<const char*>(dataPtr));
            iValuesSize.emplace_back(NumOps::SConvert<int>(dataSize));
            iValuesIsBinary.emplace_back(1);
        } break;
        case GpDbQueryValType::UUID:
        {
            const GpUUID& value = aQuery.UUID(aValueId);

            iValuesPtr.emplace_back(reinterpret_cast<const char*>(value.Data().data()));
            iValuesSize.emplace_back(NumOps::SConvert<int>(sizeof(GpUUID::DataT)));
            iValuesIsBinary.emplace_back(1);
        } break;
        case GpDbQueryValType::BLOB:
        {
            const GpBytesArray& value = aQuery.BLOB(aValueId);

            iValuesPtr.emplace_back(reinterpret_cast<const char*>(value.data()));
            iValuesSize.emplace_back(NumOps::SConvert<int>(value.size()));
            iValuesIsBinary.emplace_back(1);
        } break;
        case GpDbQueryValType::BOOLEAN:
        {
            const bool value = aQuery.Boolean(aValueId);
            iSInt64Vec.emplace_back(0);
            char* ptr = reinterpret_cast<char*>(iSInt64Vec.data() + (iSInt64Vec.size() - 1));
            *ptr = (value ? 1: 0);

            iValuesPtr.emplace_back(ptr);
            iValuesSize.emplace_back(1);
            iValuesIsBinary.emplace_back(1);
        } break;
        case GpDbQueryValType::NULL_VAL:
        {
            iValuesPtr.emplace_back(nullptr);
            iValuesSize.emplace_back(0);
            iValuesIsBinary.emplace_back(1);
        } break;
        default:
        {
            THROW_GPE("Unknown value type"_sv);
        };
    }
}

}//GPlatform

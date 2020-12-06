#include "GpDbArrayBuilder.hpp"

namespace GPlatform {

struct PgArrayHeaderT
{
//  s_int_32        vl_len_;        // varlena header
    s_int_32        dimensions;
    s_int_32        dataOffset;
    Oid             oid;
    s_int_32        elementsCount;
    s_int_32        lowerBoundary;
} __attribute__ ((__packed__));/**/

//https://github.com/postgres/postgres/blob/master/src/include/utils/array.h
/*
* A standard varlena array has the following internal structure:
 *    <vl_len_>     - standard varlena header word
 *    <ndim>        - number of dimensions of the array
 *    <dataoffset>  - offset to stored data, or 0 if no nulls bitmap
 *    <elemtype>    - element type OID
 *    <dimensions>  - length of each array axis (C array of int)
 *    <lower bnds>  - lower boundary of each dimension (C array of int)
 *    <null bitmap> - bitmap showing locations of nulls (OPTIONAL)
 *    <actual data> - whatever is the stored data
 *
 * The <dimensions> and <lower bnds> arrays each have ndim elements.
*/

std::tuple<Oid, GpBytesArray>   GpDbArrayBuilder::SBuild (const GpVector<std::string>& aArray)
{
    constexpr int TEXTOID       = 25;
    constexpr int TEXTARRAYOID  = 1009;

    GpBytesArray arrayData;
    arrayData.reserve(512);
    GpByteWriterStorageByteArray    arrayDataStorage(arrayData);
    GpByteWriter                    arrayDataWriter(arrayDataStorage);

    PgArrayHeaderT header;
    header.dimensions       = BitOps::H2N<s_int_32>(1);
    header.dataOffset       = BitOps::H2N<s_int_32>(0);
    header.oid              = BitOps::H2N<s_int_32>(TEXTOID);
    header.elementsCount    = BitOps::H2N<s_int_32>(NumOps::SConvert<s_int_32>(aArray.size()));
    header.lowerBoundary    = BitOps::H2N<s_int_32>(1);

    arrayDataWriter.Bytes({&header, 1});

    for (const std::string& e: aArray)
    {
        //Length
        arrayDataWriter.SInt32(NumOps::SConvert<s_int_32>(e.length()));

        //Data
        arrayDataWriter.Bytes(e);
    }

    return {TEXTARRAYOID, std::move(arrayData)};
}

}//namespace GPlatform

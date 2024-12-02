#pragma once
#include <tll/scheme.h>
#include "config.h"
#include <vector>

struct ParsedMessage;

struct ParsedField {
    std::string name;
    std::string tag;

    size_t size = 0;
    size_t offset = 0;

    size_t hfIndex = -1;
    size_t ettIndex = -1;

    ParsedMessage* internalMessage = nullptr;

    ParsedField* arraySize = nullptr;
    ParsedField* arrayField = nullptr;

    bool isDecimal128 = false;

    ParsedField* pointerField = nullptr;

    std::vector<ParsedField*> unionFields;

    std::vector<size_t> hfIndexesForBits;
};

struct ParsedMessage {
    std::string name;
    std::string tag;
    size_t size = 0;

    std::vector<ParsedField*> fields;

    size_t hfIndex = -1;
    size_t ettIndex = -1;
};

static tll_scheme_field_type_t
GetUnsignedIntegerForIntegerType(tll_scheme_field_type_t type)
{
    using namespace tll::scheme;
    switch(type)
    {
        case Field::Int8: return Field::UInt8;
        case Field::Int16: return Field::UInt16;
        case Field::Int32: return Field::UInt32;
        case Field::Int64: return Field::UInt64;
        default: return type;
    }
}

static uint8_t
GetBitsCountForIntegerType(tll_scheme_field_type_t type)
{
    using namespace tll::scheme;
    type = GetUnsignedIntegerForIntegerType(type);
    switch(type)
    {
        case Field::UInt8: return 8;
        case Field::UInt16: return 16;
        case Field::UInt32: return 32;
        case Field::UInt64: return 64;
        default: return 0;
    }
}

static ftenum_t
WireSharkTypeFromTLLType(tll_scheme_field_type_t type)
{
    using namespace tll::scheme;
    switch(type)
    {
        case Field::Int8: return FT_INT8;
        case Field::Int16: return FT_INT16;
        case Field::Int32: return FT_INT32;
        case Field::Int64: return FT_INT64;
        case Field::UInt8: return FT_UINT8;
        case Field::UInt16: return FT_UINT16;
        case Field::UInt32: return FT_UINT32;
        case Field::UInt64: return FT_UINT64;
        case Field::Double: return FT_DOUBLE;
        default: return FT_NONE;
    }
}

static bool
IsIntegerType(tll_scheme_field_type_t type)
{
    using namespace tll::scheme;
    return type == Field::Int8 ||
           type == Field::Int16 ||
           type == Field::Int32 ||
           type == Field::Int64 ||
           type == Field::UInt8 ||
           type == Field::UInt16 ||
           type == Field::UInt32 ||
           type == Field::UInt64;
}

static bool
IsPrimitiveType(tll_scheme_field_type_t type)
{
    using namespace tll::scheme;
    return IsIntegerType(type) || type == Field::Double;
}
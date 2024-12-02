#pragma once
#include "Utils.h"
#include <sstream>

template <class Type>
static void
UnknownFixedCallback(char *buf, Type value _U_)
{
    snprintf(buf, ITEM_LABEL_LENGTH, "Unknown fixed callback triggered");
}

template <class IntType, unsigned precision>
static void
FixedCallback(char *buf, IntType value)
{
    IntType base = 1;
    for (unsigned i = 0; i < precision; i++)
        base *= 10;

    IntType before = value / base;
    if (value < 0)
        value = -value;
    IntType after = value % base;

    std::stringstream ss;
    ss << before << '.' << after;

    snprintf(buf, ITEM_LABEL_LENGTH, "%s", ss.str().c_str());
}

template <class IntType>
static void
(*GetCallbackFunctionForFixed(unsigned precision))(char*, IntType)
{
    switch (precision)
    {
        case 1: return FixedCallback<IntType, 1>;
        case 2: return FixedCallback<IntType, 2>;
        case 3: return FixedCallback<IntType, 3>;
        case 4: return FixedCallback<IntType, 4>;
        case 5: return FixedCallback<IntType, 5>;
        case 6: return FixedCallback<IntType, 6>;
        case 7: return FixedCallback<IntType, 7>;
        case 8: return FixedCallback<IntType, 8>;
        case 9: return FixedCallback<IntType, 9>;
        case 10: return FixedCallback<IntType, 10>;
        default: return UnknownFixedCallback<IntType>;
    }
}

static void
FillCallbackFunctionForFixed(hf_register_info& hf, tll_scheme_field_type_t type, unsigned precision)
{
    using namespace tll::scheme;
    switch(type)
    {
        case Field::Int64:
            hf.hfinfo.strings = CF_FUNC(GetCallbackFunctionForFixed<int64_t>(precision));
            break;
        case Field::UInt64:
            hf.hfinfo.strings = CF_FUNC(GetCallbackFunctionForFixed<uint64_t>(precision));
            break;
        default:
            hf.hfinfo.strings = CF_FUNC(UnknownFixedCallback<int>);
            break;
    }
}
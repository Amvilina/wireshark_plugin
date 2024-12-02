#pragma once
#include "Utils.h"
#include "tll/util/time.h"

template <class Type>
static void
UnknownTimePointCallback(char *buf, Type value _U_)
{
    snprintf(buf, ITEM_LABEL_LENGTH, "Unknown time_point callback triggered");
}

template <class IntType, class resolution>
static void
TimePointCallback(char *buf, IntType value)
{
    const auto tp = tll::time_point(std::chrono::duration<IntType, resolution>(value));
    const auto stp = tll::conv::to_string(tp);
    snprintf(buf, ITEM_LABEL_LENGTH, "%s", stp.c_str());
}

template <class IntType>
static void
(*GetCallbackFunctionForTimePoint(tll_scheme_time_resolution_t resolution))(char*, IntType)
{
    switch(resolution)
    {
        case TLL_SCHEME_TIME_NS: return TimePointCallback<IntType, std::nano>;
        case TLL_SCHEME_TIME_US: return TimePointCallback<IntType, std::micro>;
        case TLL_SCHEME_TIME_MS: return TimePointCallback<IntType, std::milli>;
        case TLL_SCHEME_TIME_SECOND: return TimePointCallback<IntType, std::ratio<1, 1>>;
        case TLL_SCHEME_TIME_MINUTE: return TimePointCallback<IntType, std::ratio<60, 1>>;
        case TLL_SCHEME_TIME_HOUR: return TimePointCallback<IntType, std::ratio<3600, 1>>;
        case TLL_SCHEME_TIME_DAY: return TimePointCallback<IntType, std::ratio<86400, 1>>;
        default: return UnknownTimePointCallback<IntType>;
    }
}

static void
FillCallbackFunctionForTimePoint(hf_register_info& hf, tll_scheme_field_type_t type, tll_scheme_time_resolution_t resolution)
{
    using namespace tll::scheme;
    switch(type)
    {
        case Field::Int8:
            hf.hfinfo.strings = CF_FUNC(GetCallbackFunctionForTimePoint<int8_t>(resolution));
            break;
        case Field::Int16:
            hf.hfinfo.strings = CF_FUNC(GetCallbackFunctionForTimePoint<int16_t>(resolution));
            break;
        case Field::Int32:
            hf.hfinfo.strings = CF_FUNC(GetCallbackFunctionForTimePoint<int32_t>(resolution));
            break;
        case Field::Int64:
            hf.hfinfo.strings = CF_FUNC(GetCallbackFunctionForTimePoint<int64_t>(resolution));
            break;
        case Field::UInt8:
            hf.hfinfo.strings = CF_FUNC(GetCallbackFunctionForTimePoint<uint8_t>(resolution));
            break;
        case Field::UInt16:
            hf.hfinfo.strings = CF_FUNC(GetCallbackFunctionForTimePoint<uint16_t>(resolution));
            break;
        case Field::UInt32:
            hf.hfinfo.strings = CF_FUNC(GetCallbackFunctionForTimePoint<uint32_t>(resolution));
            break;
        case Field::UInt64:
            hf.hfinfo.strings = CF_FUNC(GetCallbackFunctionForTimePoint<uint64_t>(resolution));
            break;
        default:
            hf.hfinfo.strings = CF_FUNC(UnknownTimePointCallback<int>);
            break;
    }
}

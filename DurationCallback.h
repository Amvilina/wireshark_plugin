#pragma once
#include "Utils.h"
#include "tll/util/time.h"

template <class Type>
static void
UnknownDurationCallback(char *buf, Type value _U_)
{
    snprintf(buf, ITEM_LABEL_LENGTH, "Unknown duration callback triggered");
}

template <class Type, class Resolution>
static void
DurationCallback(char *buf, Type value)
{
    const auto d = std::chrono::duration<Type, Resolution>(value);
    const auto sd = tll::conv::to_string(d);
    snprintf(buf, ITEM_LABEL_LENGTH, "%s", sd.c_str());
}

template <class Type>
static void
(*GetCallbackFunctionForDuration(tll_scheme_time_resolution_t resolution))(char*, Type)
{
    switch(resolution) {
        case TLL_SCHEME_TIME_NS: return DurationCallback<Type, std::nano>;
        case TLL_SCHEME_TIME_US: return DurationCallback<Type, std::micro>;
        case TLL_SCHEME_TIME_MS: return DurationCallback<Type, std::milli>;
        case TLL_SCHEME_TIME_SECOND: return DurationCallback<Type, std::ratio<1, 1>>;
        case TLL_SCHEME_TIME_MINUTE: return DurationCallback<Type, std::ratio<60, 1>>;
        case TLL_SCHEME_TIME_HOUR: return DurationCallback<Type, std::ratio<3600, 1>>;
        case TLL_SCHEME_TIME_DAY: return DurationCallback<Type, std::ratio<86400, 1>>;
        default: return UnknownDurationCallback<Type>;
    }
}

static void
FillCallbackFunctionForDuration(hf_register_info& hf, tll_scheme_field_type_t type, tll_scheme_time_resolution_t resolution)
{
    using namespace tll::scheme;
    switch(type)
    {
        case Field::Int8:
            hf.hfinfo.strings = CF_FUNC(GetCallbackFunctionForDuration<int8_t>(resolution));
            break;
        case Field::Int16:
            hf.hfinfo.strings = CF_FUNC(GetCallbackFunctionForDuration<int16_t>(resolution));
            break;
        case Field::Int32:
            hf.hfinfo.strings = CF_FUNC(GetCallbackFunctionForDuration<int32_t>(resolution));
            break;
        case Field::Int64:
            hf.hfinfo.strings = CF_FUNC(GetCallbackFunctionForDuration<int64_t>(resolution));
            break;
        case Field::UInt8:
            hf.hfinfo.strings = CF_FUNC(GetCallbackFunctionForDuration<uint8_t>(resolution));
            break;
        case Field::UInt16:
            hf.hfinfo.strings = CF_FUNC(GetCallbackFunctionForDuration<uint16_t>(resolution));
            break;
        case Field::UInt32:
            hf.hfinfo.strings = CF_FUNC(GetCallbackFunctionForDuration<uint32_t>(resolution));
            break;
        case Field::UInt64:
            hf.hfinfo.strings = CF_FUNC(GetCallbackFunctionForDuration<uint64_t>(resolution));
            break;
        case Field::Double:
            hf.hfinfo.strings = CF_FUNC(GetCallbackFunctionForDuration<double>(resolution));
            break;
        default:
            hf.hfinfo.strings = CF_FUNC(UnknownDurationCallback<int>);
            break;
    }
}

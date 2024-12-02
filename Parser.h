#pragma once
#include "Utils.h"

#include "FixedCallback.h"
#include "DurationCallback.h"
#include "TimePointCallback.h"

#include "HFHandler.h"
#include "ETTHandler.h"
#include <iostream>
class Parser {
  private:
    HFHandler& _hfHandler;
    ETTHandler& _ettHandler;
  public:
    Parser(HFHandler& hfHandler, ETTHandler& ettHandler)
    : _hfHandler(hfHandler), _ettHandler(ettHandler)
    {}

    ParsedMessage* ParseMessage(tll::scheme::Message* message, const std::string& beforeTag)
    {
        ParsedMessage* parsedMessage = new ParsedMessage{};

        parsedMessage->name = message->name;
        parsedMessage->tag = (beforeTag == "") ? parsedMessage->name : beforeTag + '.' + parsedMessage->name;
        parsedMessage->size = message->size;

        hf_register_info info =
        { nullptr,
            {parsedMessage->name.c_str(), parsedMessage->tag.c_str(),
            FT_NONE, BASE_NONE,
            NULL, 0x0,
            NULL, HFILL}
        };

        parsedMessage->hfIndex = _hfHandler.AddHF(info);
        parsedMessage->ettIndex = _ettHandler.AddETT();

        for (auto field =  message->fields; field != nullptr; field = field->next) {
            ParsedField* parsedField = ParseField(field, parsedMessage->tag);
            parsedMessage->fields.push_back(parsedField);
        }

        return parsedMessage;
    }
  private:
    ParsedField* ParseField(tll::scheme::Field* field, const std::string& beforeTag)
    {
        ParsedField* parsedField = new ParsedField{};

        parsedField->name = field->name;
        parsedField->tag = beforeTag + '.' + parsedField->name;
        parsedField->size = field->size;
        parsedField->offset = field->offset;

        // Sub Types

        if (TryParseEnumType(field, parsedField))
            return parsedField;

        if (TryParseStringType(field, parsedField))
            return parsedField;

        if (TryParseTimePointType(field, parsedField))
            return parsedField;

        if (TryParseDurationType(field, parsedField))
            return parsedField;

        if (TryParseFixedType(field, parsedField))
            return parsedField;

        if (TryParseBitsType(field, parsedField))
            return parsedField;

        // Regular Types

        if (TryParseDecimal128Type(field, parsedField))
            return parsedField;

        if (TryParseBytesType(field, parsedField))
            return parsedField;

        if (TryParseMessageType(field, parsedField, beforeTag))
            return parsedField;

        if (TryParseArrayType(field, parsedField))
            return parsedField;

        if (TryParsePointerType(field, parsedField))
            return parsedField;

        if (TryParseUnionType(field, parsedField))
            return parsedField;

        if (TryParsePrimitiveType(field, parsedField))
            return parsedField;

        return parsedField;
    }

    // Sub Types

    bool TryParseEnumType(tll::scheme::Field* field, ParsedField* parsedField)
    {
        if (!IsIntegerType(field->type))
            return false;
        if (field->sub_type != tll::scheme::Field::Enum)
            return false;

        auto* val64Strings = new std::vector<val64_string>();
        for (auto nameValue = field->type_enum->values; nameValue != nullptr; nameValue = nameValue->next) {
            auto* str = new std::string(nameValue->name);
            val64_string vs = {(uint64_t)nameValue->value, str->c_str()};
            val64Strings->push_back(vs);
        }

        hf_register_info info =
        { nullptr,
            {parsedField->name.c_str(), parsedField->tag.c_str(),
            FT_INT64, BASE_DEC | BASE_VAL64_STRING,
            VALS64(val64Strings->data()), 0x0,
            NULL, HFILL}
        };

        parsedField->hfIndex = _hfHandler.AddHF(info);

        return true;
    }

    bool TryParseStringType(tll::scheme::Field* field, ParsedField* parsedField)
    {
        if (field->type != tll::scheme::Field::Bytes)
            return false;
        if (field->sub_type != tll::scheme::Field::ByteString)
            return false;

        hf_register_info info =
        { nullptr,
            {parsedField->name.c_str(), parsedField->tag.c_str(),
            FT_STRING, BASE_NONE,
            NULL, 0x0,
            NULL, HFILL}
        };
        parsedField->hfIndex = _hfHandler.AddHF(info);

        return true;
    }

    bool TryParseTimePointType(tll::scheme::Field* field, ParsedField* parsedField)
    {
        if (field->sub_type != tll::scheme::Field::TimePoint)
            return false;

        const auto wsType = WireSharkTypeFromTLLType(field->type);

        hf_register_info info =
        { nullptr,
            {parsedField->name.c_str(), parsedField->tag.c_str(),
            wsType, BASE_CUSTOM,
            NULL, 0x0,
            NULL, HFILL}
        };

        FillCallbackFunctionForTimePoint(info, field->type, field->time_resolution);

        parsedField->hfIndex = _hfHandler.AddHF(info);

        return true;
    }

    bool TryParseDurationType(tll::scheme::Field* field, ParsedField* parsedField)
    {
        if (field->sub_type != tll::scheme::Field::Duration)
            return false;

        const auto wsType = WireSharkTypeFromTLLType(field->type);

        hf_register_info info =
        { nullptr,
            {parsedField->name.c_str(), parsedField->tag.c_str(),
            wsType, BASE_CUSTOM,
            NULL, 0x0,
            NULL, HFILL}
        };

        FillCallbackFunctionForDuration(info, field->type, field->time_resolution);

        parsedField->hfIndex = _hfHandler.AddHF(info);

        return true;
    }

    bool TryParseFixedType(tll::scheme::Field* field, ParsedField* parsedField)
    {
        if (field->sub_type != tll::scheme::Field::Fixed)
            return false;

        const auto wsType = WireSharkTypeFromTLLType(field->type);

        hf_register_info info =
        { nullptr,
            {parsedField->name.c_str(), parsedField->tag.c_str(),
            wsType, BASE_CUSTOM,
            NULL, 0x0,
            NULL, HFILL}
        };

        FillCallbackFunctionForFixed(info, field->type, field->fixed_precision);

        parsedField->hfIndex = _hfHandler.AddHF(info);
        return true;
    }

    bool TryParseBitsType(tll::scheme::Field* field, ParsedField* parsedField)
    {
        if (!IsIntegerType(field->type))
            return false;
        if (field->sub_type != tll::scheme::Field::Bits)
            return false;

        const auto ws_type = WireSharkTypeFromTLLType(GetUnsignedIntegerForIntegerType(field->type));

        hf_register_info info =
        { nullptr,
            {parsedField->name.c_str(), parsedField->tag.c_str(),
            ws_type, BASE_HEX,
            NULL, 0x0,
            NULL, HFILL}
        };

        parsedField->hfIndex = _hfHandler.AddHF(info);
        parsedField->ettIndex = _ettHandler.AddETT();

        for( tll_scheme_bit_field_t* bit = field->bitfields; bit != nullptr; bit = bit->next)
        {
            std::string* bitName = new std::string(bit->name);
            std::string* bitTag = new std::string(parsedField->tag + '.' + *bitName);

            hf_register_info info =
            { nullptr,
                {bitName->c_str(), bitTag->c_str(),
                FT_BOOLEAN, GetBitsCountForIntegerType(field->type),
                NULL, (1u << bit->offset),
                NULL, HFILL}
            };

            parsedField->hfIndexesForBits.push_back(_hfHandler.AddHF(info));
        }

        return true;
    }

    // Regular Types

    bool TryParseDecimal128Type(tll::scheme::Field* field, ParsedField* parsedField)
    {
        if (field->type != tll::scheme::Field::Decimal128)
            return false;

        hf_register_info info =
        { nullptr,
            {parsedField->name.c_str(), parsedField->tag.c_str(),
            FT_NONE, BASE_NONE,
            NULL, 0x0,
            NULL, HFILL}
        };
        parsedField->hfIndex = _hfHandler.AddHF(info);

        parsedField->isDecimal128 = true;

        return true;
    }

    bool TryParseBytesType(tll::scheme::Field* field, ParsedField* parsedField)
    {
        if (field->type != tll::scheme::Field::Bytes)
            return false;

        hf_register_info info =
        { nullptr,
            {parsedField->name.c_str(), parsedField->tag.c_str(),
            FT_BYTES, SEP_COLON,
            NULL, 0x0,
            NULL, HFILL}
        };

        parsedField->hfIndex = _hfHandler.AddHF(info);

        return true;
    }

    bool TryParseMessageType(tll::scheme::Field* field, ParsedField* parsedField, const std::string& beforeTag)
    {
        if (field->type != tll::scheme::Field::Message)
            return false;
        parsedField->internalMessage = ParseMessage(field->type_msg, beforeTag);
        return true;
    }

    bool TryParseArrayType(tll::scheme::Field* field, ParsedField* parsedField)
    {
        if (field->type != tll::scheme::Field::Array)
            return false;

        hf_register_info info =
        { nullptr,
            {parsedField->name.c_str(), parsedField->tag.c_str(),
            FT_NONE, BASE_NONE,
            NULL, 0x0,
            NULL, HFILL}
        };

        parsedField->hfIndex = _hfHandler.AddHF(info);
        parsedField->ettIndex = _ettHandler.AddETT();

        parsedField->arraySize = new ParsedField{};
        parsedField->arrayField = new ParsedField{};
        parsedField->arraySize = ParseField(field->count_ptr, parsedField->tag);
        parsedField->arrayField = ParseField(field->type_array, parsedField->tag);

        return true;
    }

    bool TryParsePointerType(tll::scheme::Field* field, ParsedField* parsedField)
    {
        if (field->type != tll::scheme::Field::Pointer)
            return false;

        hf_register_info info =
        { nullptr,
            {parsedField->name.c_str(), parsedField->tag.c_str(),
            FT_NONE, BASE_NONE,
            NULL, 0x0,
            NULL, HFILL}
        };

        parsedField->hfIndex = _hfHandler.AddHF(info);
        parsedField->ettIndex = _ettHandler.AddETT();

        parsedField->pointerField = new ParsedField{};
        parsedField->pointerField = ParseField(field->type_ptr, parsedField->tag);

        return true;
    }

    bool TryParseUnionType(tll::scheme::Field* field, ParsedField* parsedField)
    {
        if (field->type != tll::scheme::Field::Union)
            return false;

        hf_register_info info =
        { nullptr,
            {parsedField->name.c_str(), parsedField->tag.c_str(),
            FT_NONE, BASE_NONE,
            NULL, 0x0,
            NULL, HFILL}
        };

        parsedField->hfIndex = _hfHandler.AddHF(info);
        parsedField->ettIndex = _ettHandler.AddETT();

        parsedField->unionFields.resize(field->type_union->fields_size);
        for (uint8_t i = 0; i < field->type_union->fields_size; i++)
        {
            parsedField->unionFields[i] = ParseField(field->type_union->fields + i, parsedField->tag);
        }

        return true;
    }

    bool TryParsePrimitiveType(tll::scheme::Field* field, ParsedField* parsedField)
    {
        if (!IsPrimitiveType(field->type))
            return false;

        const auto ws_type = WireSharkTypeFromTLLType(field->type);

        hf_register_info info =
        { nullptr,
            {parsedField->name.c_str(), parsedField->tag.c_str(),
            ws_type, BASE_DEC,
            NULL, 0x0,
            NULL, HFILL}
        };

        parsedField->hfIndex = _hfHandler.AddHF(info);

        return true;
    }
};

#pragma once
#include "Utils.h"

#include "HFHandler.h"
#include "ETTHandler.h"

#include "tll/util/decimal128.h"
#include "tll/conv/decimal128.h"

#include <tll/scheme/types.h>

class Dissector {
  private:
    HFHandler& _hfHandler;
    ETTHandler& _ettHandler;
  public:
    Dissector(HFHandler& hfHandler, ETTHandler& ettHandler)
    : _hfHandler(hfHandler), _ettHandler(ettHandler)
    {}

    void DissectMessage(tvbuff_t *tvb, proto_tree *mainTree, const ParsedMessage* parsedMessage, int offset)
    {
        const auto hf = _hfHandler.GetIntByIndex(parsedMessage->hfIndex);
        const auto ett = _ettHandler.GetIntByIndex(parsedMessage->ettIndex);
        const auto size = parsedMessage->size;

        proto_item *subItem = proto_tree_add_item(mainTree, hf, tvb, offset, size, ENC_NA);
        proto_tree *subTree = proto_item_add_subtree(subItem, ett);

        for (const auto& parsedField : parsedMessage->fields)
            DissectField(tvb, subItem, subTree, parsedField, offset);
    }

  private:

    void DissectField(tvbuff_t *tvb, proto_item *mainItem, proto_tree *mainTree, const ParsedField* parsedField, int offset)
    {
        if (TryDissectBits(tvb, mainItem, parsedField, offset))
            return;

        if (TryDissectDecimal128(tvb, mainTree, parsedField, offset))
            return;

        if (TryDissectMessage(tvb, mainTree, parsedField, offset))
            return;

        if (TryDissectArray(tvb, mainTree, parsedField, offset))
            return;

        if (TryDissectPointer(tvb, mainTree, parsedField, offset))
            return;

        if (TryDissectUnion(tvb, mainTree, parsedField, offset))
            return;

        // it's a primitive type then

        const auto hf = _hfHandler.GetIntByIndex(parsedField->hfIndex);
        const auto fieldOffset = offset + parsedField->offset;
        const auto fieldSize = parsedField->size;

        proto_tree_add_item(mainTree, hf, tvb, fieldOffset, fieldSize, ENC_LITTLE_ENDIAN);
    }

    bool TryDissectBits(tvbuff_t *tvb, proto_item *mainItem, const ParsedField* parsedField, int offset)
    {
        if (parsedField->hfIndexesForBits.size() == 0)
            return false;

        std::vector<int*> bits;
        for (const auto hfIndex : parsedField->hfIndexesForBits)
            bits.push_back(&_hfHandler.GetIntByIndex(hfIndex));
        bits.push_back(NULL);

        const auto fieldOffset = offset + parsedField->offset;
        const auto hf = _hfHandler.GetIntByIndex(parsedField->hfIndex);
        const auto ett = _ettHandler.GetIntByIndex(parsedField->ettIndex);

        proto_tree_add_bitmask(mainItem, tvb, fieldOffset, hf, ett, bits.data(), ENC_LITTLE_ENDIAN);

        return true;
    }

    bool TryDissectDecimal128(tvbuff_t *tvb, proto_tree *mainTree, const ParsedField* parsedField, int offset)
    {
        if (parsedField->isDecimal128 == false)
            return false;

        const auto hf = _hfHandler.GetIntByIndex(parsedField->hfIndex);
        const auto fieldOffset = offset + parsedField->offset;
        const auto fieldSize = parsedField->size;

        proto_item *subItem = proto_tree_add_item(mainTree, hf, tvb, fieldOffset, fieldSize, ENC_NA);

        tll_uint128_t uint128;
        uint128.lo = tvb_get_uint64(tvb, fieldOffset, ENC_LITTLE_ENDIAN);
        uint128.hi = tvb_get_uint64(tvb, fieldOffset + sizeof(uint64_t), ENC_LITTLE_ENDIAN);

        tll_decimal128_t* decimalPacked = reinterpret_cast<tll_decimal128_t*>(&uint128);
        tll_decimal128_unpacked_t* decimalUnpacked = new tll_decimal128_unpacked_t;
        tll_decimal128_unpack(decimalUnpacked, decimalPacked);

        std::string decimalStr = tll::conv::to_string(tll::util::Decimal128(*decimalUnpacked));
        proto_item_append_text(subItem, ": %s", decimalStr.c_str());

        return true;
    }

    bool TryDissectMessage(tvbuff_t *tvb, proto_tree *mainTree, const ParsedField* parsedField, int offset)
    {
        if (parsedField->internalMessage == nullptr)
            return false;

        DissectMessage(tvb, mainTree, parsedField->internalMessage, offset + parsedField->offset);

        return true;
    }

    bool TryDissectArray(tvbuff_t *tvb, proto_tree *mainTree, const ParsedField* parsedField, int offset)
    {
        if (parsedField->arrayField == nullptr)
            return false;

        const auto hf = _hfHandler.GetIntByIndex(parsedField->hfIndex);
        const auto ett = _ettHandler.GetIntByIndex(parsedField->ettIndex);

        proto_item *subItem = proto_tree_add_item(mainTree, hf, tvb, offset, parsedField->size, ENC_NA);
        proto_tree *subTree = proto_item_add_subtree(subItem, ett);

        uint64_t count = GetArrayCount(tvb, parsedField, offset);
        proto_item_append_text(subItem, " [size %lu]", count);

        for (uint64_t i = 0; i < count; i++) {
            const auto fieldOffset = offset + parsedField->offset + i * parsedField->arrayField->size;
            DissectField(tvb, subItem, subTree, parsedField->arrayField, fieldOffset);
        }

        return true;
    }

    uint64_t GetArrayCount(tvbuff_t *tvb, const ParsedField* parsedField, int offset)
    {
        const auto fieldOffset = offset + parsedField->offset + parsedField->arraySize->offset;
        switch (parsedField->arraySize->size)
        {
            case 1: return tvb_get_uint8(tvb, fieldOffset);
            case 2: return tvb_get_uint16(tvb, fieldOffset, ENC_LITTLE_ENDIAN);
            case 4: return tvb_get_uint32(tvb, fieldOffset, ENC_LITTLE_ENDIAN);
            case 8: return tvb_get_uint64(tvb, fieldOffset, ENC_LITTLE_ENDIAN);
            default: return 0;
        }
    }

    bool TryDissectPointer(tvbuff_t *tvb, proto_tree *mainTree, const ParsedField* parsedField, int offset)
    {
        if (parsedField->pointerField == nullptr)
            return false;

        const auto hf = _hfHandler.GetIntByIndex(parsedField->hfIndex);
        const auto ett = _ettHandler.GetIntByIndex(parsedField->ettIndex);

        proto_item *subItem = proto_tree_add_item(mainTree, hf, tvb, offset, parsedField->size, ENC_NA);
        proto_tree *subTree = proto_item_add_subtree(subItem, ett);

        uint64_t pointerValue = tvb_get_uint64(tvb, offset + parsedField->offset, ENC_LITTLE_ENDIAN);
        tll_scheme_offset_ptr_t* offsetPtr = reinterpret_cast<tll_scheme_offset_ptr_t*>(&pointerValue);

        proto_item_append_text(subItem, " [size %u]", offsetPtr->size);

        for (uint32_t i = 0; i < offsetPtr->size; i++) {
            const auto fieldOffset = offset + parsedField->offset + offsetPtr->offset + i * parsedField->pointerField->size;
            DissectField(tvb, subItem, subTree, parsedField->pointerField, fieldOffset);
        }

        return true;
    }

    bool TryDissectUnion(tvbuff_t *tvb, proto_tree *mainTree, const ParsedField* parsedField, int offset)
    {
        if (parsedField->unionFields.size() == 0)
            return false;

        const auto hf = _hfHandler.GetIntByIndex(parsedField->hfIndex);
        const auto ett = _ettHandler.GetIntByIndex(parsedField->ettIndex);

        proto_item *subItem = proto_tree_add_item(mainTree, hf, tvb, offset, parsedField->size, ENC_NA);
        proto_tree *subTree = proto_item_add_subtree(subItem, ett);

        const auto fieldOffset = offset + parsedField->offset;

        uint8_t tagValue = tvb_get_uint8(tvb, fieldOffset);
        proto_item_append_text(subItem, " [tag %u]", tagValue);

        ParsedField* innerField = parsedField->unionFields[tagValue];

        DissectField(tvb, subItem, subTree, innerField, fieldOffset);

        return true;
    }
};
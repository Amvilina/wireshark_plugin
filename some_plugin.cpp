#include <epan/packet.h>

#include "LoadScheme.h"
#include "Parser.h"
#include "Dissector.h"

#include <unordered_map>

static int gProto;

static size_t gETTProtoIndex;
static size_t gETTFrameIndex;

static size_t gHFFrameIndex;
static size_t gHFFrameSizeIndex;
static size_t gHFFrameMsgidIndex;
static size_t gHFFrameSeqIndex;

static HFHandler hfHandler;
static ETTHandler ettHandler;

using namespace tll::scheme;

std::unordered_map<int32_t, ParsedMessage*> gParsedMessages;

static int
Dissect(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data _U_)
{
    col_clear(pinfo->cinfo,COL_INFO);
    col_set_str(pinfo->cinfo, COL_PROTOCOL, "s_p");

    int offset = 0;
    proto_item *mainItem = proto_tree_add_item(tree, gProto, tvb, 0, -1, ENC_NA);
    proto_tree *mainTree = proto_item_add_subtree(mainItem, ettHandler.GetIntByIndex(gETTProtoIndex));

    proto_item *frameItem = proto_tree_add_item(mainTree, hfHandler.GetIntByIndex(gHFFrameIndex), tvb, offset, 16, ENC_NA);
    proto_tree *frameTree = proto_item_add_subtree(frameItem, ettHandler.GetIntByIndex(gETTFrameIndex));

    proto_tree_add_item(frameTree, hfHandler.GetIntByIndex(gHFFrameSizeIndex), tvb, offset, 4, ENC_LITTLE_ENDIAN);
    uint32_t size = tvb_get_uint32(tvb, offset, ENC_LITTLE_ENDIAN);
    offset += 4;

    proto_tree_add_item(frameTree, hfHandler.GetIntByIndex(gHFFrameMsgidIndex), tvb, offset, 4, ENC_LITTLE_ENDIAN);
    int32_t msgid = tvb_get_int32(tvb, offset, ENC_LITTLE_ENDIAN);
    offset += 4;

    proto_tree_add_item(frameTree, hfHandler.GetIntByIndex(gHFFrameSeqIndex), tvb, offset, 8, ENC_LITTLE_ENDIAN);
    int64_t seq = tvb_get_int64(tvb, offset, ENC_LITTLE_ENDIAN);
    offset += 8;

    col_add_fstr(pinfo->cinfo, COL_INFO, " msgid=%d size=%d seq=%ld", msgid, size, seq);

    Dissector dissector(hfHandler, ettHandler);

    if (gParsedMessages.count(msgid) > 0) {
        dissector.DissectMessage(tvb, mainTree, gParsedMessages[msgid], offset);
    }

    return tvb_captured_length(tvb);
}

static void
ParseScheme()
{
    auto s = LoadScheme();
    Parser parser(hfHandler, ettHandler);
    for (auto m = s->messages; m != nullptr; m = m->next) {
        if (gParsedMessages.count(m->msgid) > 0)
            continue;
        ParsedMessage* parsedMessage = parser.ParseMessage(m, "s_p");
        gParsedMessages.insert({m->msgid, parsedMessage});
    }
}

void RegisterProtocol()
{
    hf_register_info info;

    info = { nullptr,
                {"frame", "s_p.frame",
                FT_NONE, BASE_NONE,
                NULL, 0x0,
                NULL, HFILL}
            };
    gHFFrameIndex = hfHandler.AddHF(info);

    info = { nullptr,
                {"size", "s_p.frame.size",
                FT_UINT32, BASE_DEC,
                NULL, 0x0,
                NULL, HFILL}
            };
    gHFFrameSizeIndex = hfHandler.AddHF(info);

    info = { nullptr,
                {"msgid", "s_p.frame.msgid",
                FT_INT32, BASE_DEC,
                NULL, 0x0,
                NULL, HFILL}
            };
    gHFFrameMsgidIndex = hfHandler.AddHF(info);

    info = { nullptr,
                {"seq", "s_p.frame.seq",
                FT_INT64, BASE_DEC,
                NULL, 0x0,
                NULL, HFILL}
            };
    gHFFrameSeqIndex = hfHandler.AddHF(info);

    gETTProtoIndex = ettHandler.AddETT();
    gETTFrameIndex = ettHandler.AddETT();

    ParseScheme();
    gProto = proto_register_protocol("SOME_PLUGIN", "some_plugin", "s_p");

    hfHandler.LinkHF();
    ettHandler.LinkETT();

    proto_register_field_array(gProto, hfHandler.GetData(), guint(hfHandler.Size()));
    proto_register_subtree_array(ettHandler.GetData(), guint(ettHandler.Size()));
}

void HandoffProtocol()
{
    static dissector_handle_t tll_handle;

    tll_handle = create_dissector_handle(Dissect, gProto);

    dissector_add_uint_range_with_preference("udp.port", "14000-15000", tll_handle);

    dissector_add_uint_range_with_preference("tcp.port", "14000-15000", tll_handle);
    dissector_add_uint_range_with_preference("tcp.port", "52500-52600", tll_handle);
}

extern "C"
{

void proto_register_some_plugin(void)
{
    RegisterProtocol();
}

void proto_reg_handoff_some_plugin(void)
{
    HandoffProtocol();
}

}
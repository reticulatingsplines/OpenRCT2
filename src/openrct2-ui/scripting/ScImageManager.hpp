/*****************************************************************************
 * Copyright (c) 2014-2021 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#ifdef ENABLE_SCRIPTING

#    include <openrct2/drawing/Image.h>
#    include <openrct2/scripting/Duktape.hpp>
#    include <openrct2/sprites.h>

namespace OpenRCT2::Scripting
{
    class ScImageManager
    {
    private:
        duk_context* _ctx{};

    public:
        ScImageManager(duk_context* ctx)
            : _ctx(ctx)
        {
        }

        static void Register(duk_context* ctx)
        {
            dukglue_register_method(ctx, &ScImageManager::getPredefinedRange, "getPredefinedRange");
            dukglue_register_method(ctx, &ScImageManager::getAvailableAllocationRanges, "getAvailableAllocationRanges");
        }

    private:
        DukValue getPredefinedRange(const std::string& name) const
        {
            if (name == "g1")
            {
                return CreateImageIndexRange(0, SPR_G1_END);
            }
            else if (name == "g2")
            {
                return CreateImageIndexRange(SPR_G2_BEGIN, SPR_G2_END - SPR_G2_BEGIN);
            }
            else if (name == "csg")
            {
                return CreateImageIndexRange(SPR_CSG_BEGIN, SPR_CSG_END - SPR_CSG_BEGIN);
            }
            else if (name == "allocated")
            {
                return CreateImageIndexRange(SPR_IMAGE_LIST_BEGIN, SPR_IMAGE_LIST_END - SPR_IMAGE_LIST_BEGIN);
            }
            else
            {
                return ToDuk(_ctx, nullptr);
            }
        }

        DukValue getAvailableAllocationRanges() const
        {
            auto ranges = GetAvailableAllocationRanges();
            duk_push_array(_ctx);
            duk_uarridx_t index = 0;
            for (const auto& range : ranges)
            {
                auto value = CreateImageIndexRange(range.BaseId, range.Count);
                value.push();
                duk_put_prop_index(_ctx, /* duk stack index */ -2, index);
                index++;
            }
            return DukValue::take_from_stack(_ctx);
        }

        DukValue CreateImageIndexRange(size_t start, size_t count) const
        {
            DukObject obj(_ctx);
            obj.Set("start", static_cast<uint32_t>(start));
            obj.Set("count", static_cast<uint32_t>(count));
            return obj.Take();
        }
    };
} // namespace OpenRCT2::Scripting

#endif

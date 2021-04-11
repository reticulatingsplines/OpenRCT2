/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include "../common.h"
#include "../core/FixedVector.h"
#include "../drawing/Drawing.h"
#include "../interface/Colour.h"
#include "../world/Location.hpp"

struct TileElement;
enum class ViewportInteractionItem : uint8_t;

#pragma pack(push, 1)
struct attached_paint_struct
{
    uint32_t image_id;
    union
    {
        uint32_t tertiary_colour;
        // If masked image_id is masked_id
        uint32_t colour_image_id;
    };
    int32_t x;
    int32_t y;
    uint8_t flags;
    uint8_t pad_0D;
    attached_paint_struct* next;
};
#ifdef PLATFORM_32BIT
// TODO: drop packing from this when all rendering is done.
assert_struct_size(attached_paint_struct, 0x12);
#endif

enum PAINT_QUADRANT_FLAGS
{
    PAINT_QUADRANT_FLAG_IDENTICAL = (1 << 0),
    PAINT_QUADRANT_FLAG_BIGGER = (1 << 7),
    PAINT_QUADRANT_FLAG_NEXT = (1 << 1),
};

struct paint_struct_bound_box
{
    int32_t x;
    int32_t y;
    int32_t z;
    int32_t x_end;
    int32_t y_end;
    int32_t z_end;
};

struct paint_struct
{
    uint32_t image_id;
    union
    {
        uint32_t tertiary_colour;
        // If masked image_id is masked_id
        uint32_t colour_image_id;
    };
    paint_struct_bound_box bounds;
    int32_t x;
    int32_t y;
    uint16_t quadrant_index;
    uint8_t flags;
    uint8_t quadrant_flags;
    attached_paint_struct* attached_ps;
    paint_struct* children;
    paint_struct* next_quadrant_ps;
    ViewportInteractionItem sprite_type;
    uint8_t var_29;
    uint16_t pad_2A;
    int32_t map_x;
    int32_t map_y;
    TileElement* tileElement;
};
#ifdef PLATFORM_32BIT
// TODO: drop packing from this when all rendering is done.
assert_struct_size(paint_struct, 0x34);
#endif

struct paint_string_struct
{
    rct_string_id string_id;   // 0x00
    paint_string_struct* next; // 0x02
    int32_t x;                 // 0x06
    int32_t y;                 // 0x08
    uint32_t args[4];          // 0x0A
    uint8_t* y_offsets;        // 0x1A
};
#pragma pack(pop)

union paint_entry
{
    paint_struct basic;
    attached_paint_struct attached;
    paint_string_struct string;
};

struct sprite_bb
{
    uint32_t sprite_id;
    LocationXYZ16 offset;
    LocationXYZ16 bb_offset;
    LocationXYZ16 bb_size;
};

enum PAINT_STRUCT_FLAGS
{
    PAINT_STRUCT_FLAG_IS_MASKED = (1 << 0)
};

struct support_height
{
    uint16_t height;
    uint8_t slope;
    uint8_t pad;
};

struct tunnel_entry
{
    uint8_t height;
    uint8_t type;
};

#define MAX_PAINT_QUADRANTS 512
#define TUNNEL_MAX_COUNT 65

struct paint_session
{
    rct_drawpixelinfo DPI;
    FixedVector<paint_entry, 4000> PaintStructs;
    paint_struct* Quadrants[MAX_PAINT_QUADRANTS];
    paint_struct* LastPS;
    paint_string_struct* PSStringHead;
    paint_string_struct* LastPSString;
    attached_paint_struct* LastAttachedPS;
    const TileElement* SurfaceElement;
    const void* CurrentlyDrawnItem;
    TileElement* PathElementOnSameHeight;
    TileElement* TrackElementOnSameHeight;
    paint_struct PaintHead;
    uint32_t ViewFlags;
    uint32_t QuadrantBackIndex;
    uint32_t QuadrantFrontIndex;
    CoordsXY SpritePosition;
    ViewportInteractionItem InteractionType;
    uint8_t CurrentRotation;
    support_height SupportSegments[9];
    support_height Support;
    paint_struct* WoodenSupportsPrependTo;
    CoordsXY MapPosition;
    tunnel_entry LeftTunnels[TUNNEL_MAX_COUNT];
    uint8_t LeftTunnelCount;
    tunnel_entry RightTunnels[TUNNEL_MAX_COUNT];
    uint8_t RightTunnelCount;
    uint8_t VerticalTunnelHeight;
    bool DidPassSurface;
    uint8_t Unk141E9DB;
    uint16_t WaterHeight;
    uint32_t TrackColours[4];

    constexpr bool NoPaintStructsAvailable() noexcept
    {
        return PaintStructs.size() >= PaintStructs.capacity();
    }

    constexpr paint_struct* AllocateNormalPaintEntry() noexcept
    {
        LastPS = &PaintStructs.emplace_back().basic;
        return LastPS;
    }

    constexpr attached_paint_struct* AllocateAttachedPaintEntry() noexcept
    {
        LastAttachedPS = &PaintStructs.emplace_back().attached;
        return LastAttachedPS;
    }

    constexpr paint_string_struct* AllocateStringPaintEntry() noexcept
    {
        auto* string = &PaintStructs.emplace_back().string;
        if (LastPSString == nullptr)
        {
            PSStringHead = string;
        }
        else
        {
            LastPSString->next = string;
        }
        LastPSString = string;
        return LastPSString;
    }
};

extern paint_session gPaintSession;

// Globals for paint clipping
extern uint8_t gClipHeight;
extern CoordsXY gClipSelectionA;
extern CoordsXY gClipSelectionB;

/** rct2: 0x00993CC4. The white ghost that indicates not-yet-built elements. */
#define CONSTRUCTION_MARKER (COLOUR_DARK_GREEN << 19 | COLOUR_GREY << 24 | IMAGE_TYPE_REMAP)
extern bool gShowDirtyVisuals;
extern bool gPaintBoundingBoxes;
extern bool gPaintBlockedTiles;
extern bool gPaintWidePathsAsGhost;

paint_struct* PaintAddImageAsParent(
    paint_session* session, uint32_t image_id, int32_t x_offset, int32_t y_offset, int32_t bound_box_length_x,
    int32_t bound_box_length_y, int32_t bound_box_length_z, int32_t z_offset);
paint_struct* PaintAddImageAsParent(
    paint_session* session, uint32_t image_id, const CoordsXYZ& offset, const CoordsXYZ& boundBoxSize);
paint_struct* PaintAddImageAsParent(
    paint_session* session, uint32_t image_id, int32_t x_offset, int32_t y_offset, int32_t bound_box_length_x,
    int32_t bound_box_length_y, int32_t bound_box_length_z, int32_t z_offset, int32_t bound_box_offset_x,
    int32_t bound_box_offset_y, int32_t bound_box_offset_z);
[[nodiscard]] paint_struct* PaintAddImageAsOrphan(
    paint_session* session, uint32_t image_id, int32_t x_offset, int32_t y_offset, int32_t bound_box_length_x,
    int32_t bound_box_length_y, int32_t bound_box_length_z, int32_t z_offset, int32_t bound_box_offset_x,
    int32_t bound_box_offset_y, int32_t bound_box_offset_z);
paint_struct* PaintAddImageAsChild(
    paint_session* session, uint32_t image_id, int32_t x_offset, int32_t y_offset, int32_t bound_box_length_x,
    int32_t bound_box_length_y, int32_t bound_box_length_z, int32_t z_offset, int32_t bound_box_offset_x,
    int32_t bound_box_offset_y, int32_t bound_box_offset_z);
paint_struct* PaintAddImageAsChild(
    paint_session* session, uint32_t image_id, const CoordsXYZ& offset, const CoordsXYZ& boundBoxLength,
    const CoordsXYZ& boundBoxOffset);

paint_struct* PaintAddImageAsParentRotated(
    paint_session* session, uint8_t direction, uint32_t image_id, int32_t x_offset, int32_t y_offset,
    int32_t bound_box_length_x, int32_t bound_box_length_y, int32_t bound_box_length_z, int32_t z_offset);
paint_struct* PaintAddImageAsParentRotated(
    paint_session* session, uint8_t direction, uint32_t image_id, int32_t x_offset, int32_t y_offset,
    int32_t bound_box_length_x, int32_t bound_box_length_y, int32_t bound_box_length_z, int32_t z_offset,
    int32_t bound_box_offset_x, int32_t bound_box_offset_y, int32_t bound_box_offset_z);
paint_struct* PaintAddImageAsChildRotated(
    paint_session* session, uint8_t direction, uint32_t image_id, int32_t x_offset, int32_t y_offset,
    int32_t bound_box_length_x, int32_t bound_box_length_y, int32_t bound_box_length_z, int32_t z_offset,
    int32_t bound_box_offset_x, int32_t bound_box_offset_y, int32_t bound_box_offset_z);

void paint_util_push_tunnel_rotated(paint_session* session, uint8_t direction, uint16_t height, uint8_t type);

bool PaintAttachToPreviousAttach(paint_session* session, uint32_t image_id, int32_t x, int32_t y);
bool PaintAttachToPreviousPS(paint_session* session, uint32_t image_id, int32_t x, int32_t y);
void PaintFloatingMoneyEffect(
    paint_session* session, money64 amount, rct_string_id string_id, int32_t y, int32_t z, int8_t y_offsets[], int32_t offset_x,
    uint32_t rotation);

paint_session* PaintSessionAlloc(rct_drawpixelinfo* dpi, uint32_t viewFlags);
void PaintSessionFree(paint_session* session);
void PaintSessionGenerate(paint_session* session);
void PaintSessionArrange(paint_session* session);
void PaintDrawStructs(paint_session* session);
void PaintDrawMoneyStructs(rct_drawpixelinfo* dpi, paint_string_struct* ps);

// TESTING
#ifdef __TESTPAINT__
void testpaint_clear_ignore();
void testpaint_ignore(uint8_t direction, uint8_t trackSequence);
void testpaint_ignore_all();
bool testpaint_is_ignored(uint8_t direction, uint8_t trackSequence);

#    define TESTPAINT_IGNORE(direction, trackSequence) testpaint_ignore(direction, trackSequence)
#    define TESTPAINT_IGNORE_ALL() testpaint_ignore_all()
#else
#    define TESTPAINT_IGNORE(direction, trackSequence)
#    define TESTPAINT_IGNORE_ALL()
#endif

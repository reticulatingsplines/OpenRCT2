/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma warning(disable : 4706) // assignment within conditional expression

#include "LargeSceneryObject.h"

#include "../core/IStream.hpp"
#include "../core/Json.hpp"
#include "../core/Memory.hpp"
#include "../drawing/Drawing.h"
#include "../drawing/Image.h"
#include "../interface/Cursors.h"
#include "../localisation/Language.h"
#include "../world/Banner.h"
#include "../world/Location.hpp"

#include <algorithm>
#include <iterator>

void LargeSceneryObject::ReadLegacy(IReadObjectContext* context, OpenRCT2::IStream* stream)
{
    stream->Seek(6, OpenRCT2::STREAM_SEEK_CURRENT);
    _legacyType.tool_id = static_cast<CursorID>(stream->ReadValue<uint8_t>());
    _legacyType.flags = stream->ReadValue<uint8_t>();
    _legacyType.price = stream->ReadValue<int16_t>();
    _legacyType.removal_price = stream->ReadValue<int16_t>();
    stream->Seek(5, OpenRCT2::STREAM_SEEK_CURRENT);
    _legacyType.scenery_tab_id = OBJECT_ENTRY_INDEX_NULL;
    _legacyType.scrolling_mode = stream->ReadValue<uint8_t>();
    stream->Seek(4, OpenRCT2::STREAM_SEEK_CURRENT);

    GetStringTable().Read(context, stream, ObjectStringID::NAME);

    rct_object_entry sgEntry = stream->ReadValue<rct_object_entry>();
    SetPrimarySceneryGroup(ObjectEntryDescriptor(sgEntry));

    if (_legacyType.flags & LARGE_SCENERY_FLAG_3D_TEXT)
    {
        rct_large_scenery_text _3dFontLegacy = {};
        stream->Read(&_3dFontLegacy);
        _3dFont = std::make_unique<LargeSceneryText>(_3dFontLegacy);
        _legacyType.text = _3dFont.get();
    }

    _tiles = ReadTiles(stream);

    GetImageTable().Read(context, stream);

    // Validate properties
    if (_legacyType.price <= 0)
    {
        context->LogError(ObjectError::InvalidProperty, "Price can not be free or negative.");
    }
    if (_legacyType.removal_price <= 0)
    {
        // Make sure you don't make a profit when placing then removing.
        money16 reimbursement = _legacyType.removal_price;
        if (reimbursement > _legacyType.price)
        {
            context->LogError(ObjectError::InvalidProperty, "Sell price can not be more than buy price.");
        }
    }
}

void LargeSceneryObject::Load()
{
    GetStringTable().Sort();
    _legacyType.name = language_allocate_object_string(GetName());
    _baseImageId = gfx_object_allocate_images(GetImageTable().GetImages(), GetImageTable().GetCount());
    _legacyType.image = _baseImageId;

    _legacyType.tiles = _tiles.data();

    if (_legacyType.flags & LARGE_SCENERY_FLAG_3D_TEXT)
    {
        _legacyType.text_image = _legacyType.image;
        if (_3dFont->flags & LARGE_SCENERY_TEXT_FLAG_VERTICAL)
        {
            _legacyType.image += _3dFont->num_images * 2;
        }
        else
        {
            _legacyType.image += _3dFont->num_images * 4;
        }
        _legacyType.text = _3dFont.get();
    }
}

void LargeSceneryObject::Unload()
{
    language_free_object_string(_legacyType.name);
    gfx_object_free_images(_baseImageId, GetImageTable().GetCount());

    _legacyType.name = 0;
    _baseImageId = _legacyType.image = 0;
}

void LargeSceneryObject::DrawPreview(rct_drawpixelinfo* dpi, int32_t width, int32_t height) const
{
    auto screenCoords = ScreenCoordsXY{ width / 2, (height / 2) - 39 };

    const auto image = ImageId(_legacyType.image, COLOUR_BORDEAUX_RED, COLOUR_YELLOW);
    gfx_draw_sprite(dpi, image, screenCoords);
}

std::vector<rct_large_scenery_tile> LargeSceneryObject::ReadTiles(OpenRCT2::IStream* stream)
{
    auto tiles = std::vector<rct_large_scenery_tile>();
    while (stream->ReadValue<uint16_t>() != 0xFFFF)
    {
        stream->Seek(-2, OpenRCT2::STREAM_SEEK_CURRENT);
        auto tile = stream->ReadValue<rct_large_scenery_tile>();
        tiles.push_back(std::move(tile));
    }
    tiles.push_back({ -1, -1, -1, 255, 0xFFFF });
    return tiles;
}

void LargeSceneryObject::ReadJson(IReadObjectContext* context, json_t& root)
{
    Guard::Assert(root.is_object(), "LargeSceneryObject::ReadJson expects parameter root to be object");

    auto properties = root["properties"];

    if (properties.is_object())
    {
        _legacyType.tool_id = Cursor::FromString(Json::GetString(properties["cursor"]), CursorID::StatueDown);

        _legacyType.price = Json::GetNumber<int16_t>(properties["price"]);
        _legacyType.removal_price = Json::GetNumber<int16_t>(properties["removalPrice"]);

        _legacyType.scrolling_mode = Json::GetNumber<uint8_t>(properties["scrollingMode"], SCROLLING_MODE_NONE);

        _legacyType.flags = Json::GetFlags<uint8_t>(
            properties,
            {
                { "hasPrimaryColour", LARGE_SCENERY_FLAG_HAS_PRIMARY_COLOUR },
                { "hasSecondaryColour", LARGE_SCENERY_FLAG_HAS_SECONDARY_COLOUR },
                { "isAnimated", LARGE_SCENERY_FLAG_ANIMATED },
                { "isPhotogenic", LARGE_SCENERY_FLAG_PHOTOGENIC },
                { "isTree", LARGE_SCENERY_FLAG_IS_TREE },
            });

        // Tiles
        auto jTiles = properties["tiles"];
        if (jTiles.is_array())
        {
            _tiles = ReadJsonTiles(jTiles);
        }

        // Read text
        auto j3dFont = properties["3dFont"];
        if (j3dFont.is_object())
        {
            _3dFont = ReadJson3dFont(j3dFont);
            _legacyType.flags |= LARGE_SCENERY_FLAG_3D_TEXT;
        }

        SetPrimarySceneryGroup(ObjectEntryDescriptor(Json::GetString(properties["sceneryGroup"])));
    }

    PopulateTablesFromJson(context, root);
}

std::vector<rct_large_scenery_tile> LargeSceneryObject::ReadJsonTiles(json_t& jTiles)
{
    std::vector<rct_large_scenery_tile> tiles;

    for (auto& jTile : jTiles)
    {
        if (jTile.is_object())
        {
            rct_large_scenery_tile tile = {};
            tile.x_offset = Json::GetNumber<int16_t>(jTile["x"]);
            tile.y_offset = Json::GetNumber<int16_t>(jTile["y"]);
            tile.z_offset = Json::GetNumber<int16_t>(jTile["z"]);
            tile.z_clearance = Json::GetNumber<int8_t>(jTile["clearance"]);

            // clang-format off
            tile.flags = Json::GetFlags<uint16_t>(
                jTile,
                {
                    {"hasSupports",         LARGE_SCENERY_TILE_FLAG_NO_SUPPORTS,            Json::FlagType::Inverted},
                    {"allowSupportsAbove",  LARGE_SCENERY_TILE_FLAG_ALLOW_SUPPORTS_ABOVE,   Json::FlagType::Normal}
                });
            // clang-format on

            // All corners are by default occupied
            uint16_t corners = Json::GetNumber<uint16_t>(jTile["corners"], 0xF);
            tile.flags |= (corners & 0xFF) << 12;

            auto walls = Json::GetNumber<int16_t>(jTile["walls"]);
            tile.flags |= (walls & 0xFF) << 8;

            tiles.push_back(std::move(tile));
        }
    }

    // HACK Add end of tiles marker
    //      We should remove this later by improving the code base to use tiles array length
    tiles.push_back({ -1, -1, -1, 0xFF, 0xFFFF });

    return tiles;
}

std::unique_ptr<LargeSceneryText> LargeSceneryObject::ReadJson3dFont(json_t& j3dFont)
{
    Guard::Assert(j3dFont.is_object(), "LargeSceneryObject::ReadJson3dFont expects parameter j3dFont to be object");

    auto font = std::make_unique<LargeSceneryText>();

    auto jOffsets = j3dFont["offsets"];
    if (jOffsets.is_array())
    {
        auto offsets = ReadJsonOffsets(jOffsets);
        auto numOffsets = std::min(std::size(font->offset), offsets.size());
        std::copy_n(offsets.data(), numOffsets, font->offset);
    }

    font->max_width = Json::GetNumber<uint16_t>(j3dFont["maxWidth"]);
    font->num_images = Json::GetNumber<uint16_t>(j3dFont["numImages"]);

    font->flags = Json::GetFlags<uint8_t>(
        j3dFont,
        {
            { "isVertical", LARGE_SCENERY_TEXT_FLAG_VERTICAL },
            { "isTwoLine", LARGE_SCENERY_TEXT_FLAG_TWO_LINE },
        });

    auto jGlyphs = j3dFont["glyphs"];
    if (jGlyphs.is_array())
    {
        auto glyphs = ReadJsonGlyphs(jGlyphs);
        auto numGlyphs = std::min(std::size(font->glyphs), glyphs.size());
        std::copy_n(glyphs.data(), numGlyphs, font->glyphs);
    }

    return font;
}

std::vector<CoordsXY> LargeSceneryObject::ReadJsonOffsets(json_t& jOffsets)
{
    std::vector<CoordsXY> offsets;
    for (auto& jOffset : jOffsets)
    {
        if (jOffset.is_object())
        {
            CoordsXY offset = {};
            offset.x = Json::GetNumber<int16_t>(jOffset["x"]);
            offset.y = Json::GetNumber<int16_t>(jOffset["y"]);
            offsets.push_back(offset);
        }
    }
    return offsets;
}

std::vector<rct_large_scenery_text_glyph> LargeSceneryObject::ReadJsonGlyphs(json_t& jGlyphs)
{
    std::vector<rct_large_scenery_text_glyph> glyphs;
    for (auto& jGlyph : jGlyphs)
    {
        if (jGlyph.is_object())
        {
            rct_large_scenery_text_glyph glyph = {};
            glyph.image_offset = Json::GetNumber<uint8_t>(jGlyph["image"]);
            glyph.width = Json::GetNumber<uint8_t>(jGlyph["width"]);
            glyph.height = Json::GetNumber<uint8_t>(jGlyph["height"]);
            glyphs.push_back(glyph);
        }
    }
    return glyphs;
}

const rct_large_scenery_tile* LargeSceneryObject::GetTileForSequence(uint8_t SequenceIndex) const
{
    if (SequenceIndex >= _tiles.size())
        return nullptr;

    return &_tiles[SequenceIndex];
}

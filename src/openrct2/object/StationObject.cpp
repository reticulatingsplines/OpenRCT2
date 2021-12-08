/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "StationObject.h"

#include "../core/IStream.hpp"
#include "../core/Json.hpp"
#include "../core/String.hpp"
#include "../drawing/Drawing.h"
#include "../drawing/Image.h"
#include "../localisation/Localisation.h"
#include "../world/Banner.h"

void StationObject::Load()
{
    GetStringTable().Sort();
    NameStringId = language_allocate_object_string(GetName());

    auto numImages = GetImageTable().GetCount();
    if (numImages != 0)
    {
        BaseImageId = gfx_object_allocate_images(GetImageTable().GetImages(), GetImageTable().GetCount());

        uint32_t shelterOffset = (Flags & STATION_OBJECT_FLAGS::IS_TRANSPARENT) ? 32 : 16;
        if (numImages > shelterOffset)
        {
            ShelterImageId = BaseImageId + shelterOffset;
        }
    }
}

void StationObject::Unload()
{
    language_free_object_string(NameStringId);
    gfx_object_free_images(BaseImageId, GetImageTable().GetCount());

    NameStringId = 0;
    BaseImageId = 0;
    ShelterImageId = 0;
}

void StationObject::DrawPreview(rct_drawpixelinfo* dpi, int32_t width, int32_t height) const
{
    auto screenCoords = ScreenCoordsXY{ width / 2, (height / 2) + 16 };

    auto colour0 = COLOUR_LIGHT_BLUE;
    auto colour1 = COLOUR_BORDEAUX_RED;
    auto tcolour0 = EnumValue(GlassPaletteIds[colour0]);

    uint32_t imageId = BaseImageId;
    uint32_t tImageId = BaseImageId + 16;
    if (Flags & STATION_OBJECT_FLAGS::HAS_PRIMARY_COLOUR)
    {
        imageId |= (colour0 << 19) | IMAGE_TYPE_REMAP;
        tImageId |= (EnumValue(GlassPaletteIds[tcolour0]) << 19) | IMAGE_TYPE_TRANSPARENT;
    }
    if (Flags & STATION_OBJECT_FLAGS::HAS_SECONDARY_COLOUR)
    {
        imageId |= (colour1 << 24) | IMAGE_TYPE_REMAP_2_PLUS;
        tImageId |= (colour1 << 24) | IMAGE_TYPE_REMAP_2_PLUS;
    }

    gfx_draw_sprite(dpi, imageId + 0, screenCoords, 0);
    if (Flags & STATION_OBJECT_FLAGS::IS_TRANSPARENT)
    {
        gfx_draw_sprite(dpi, tImageId, screenCoords, 0);
    }

    gfx_draw_sprite(dpi, imageId + 4, screenCoords, 0);
    if (Flags & STATION_OBJECT_FLAGS::IS_TRANSPARENT)
    {
        gfx_draw_sprite(dpi, tImageId + 4, screenCoords, 0);
    }
}

void StationObject::ReadJson(IReadObjectContext* context, json_t& root)
{
    Guard::Assert(root.is_object(), "StationObject::ReadJson expects parameter root to be object");

    auto properties = root["properties"];

    if (properties.is_object())
    {
        Height = Json::GetNumber<int32_t>(properties["height"]);
        ScrollingMode = Json::GetNumber<uint8_t>(properties["scrollingMode"], SCROLLING_MODE_NONE);
        Flags = Json::GetFlags<uint32_t>(
            properties,
            {
                { "hasPrimaryColour", STATION_OBJECT_FLAGS::HAS_PRIMARY_COLOUR },
                { "hasSecondaryColour", STATION_OBJECT_FLAGS::HAS_SECONDARY_COLOUR },
                { "isTransparent", STATION_OBJECT_FLAGS::IS_TRANSPARENT },
                { "noPlatforms", STATION_OBJECT_FLAGS::NO_PLATFORMS },
                { "hasShelter", STATION_OBJECT_FLAGS::HAS_SHELTER },
            });
    }

    PopulateTablesFromJson(context, root);
}

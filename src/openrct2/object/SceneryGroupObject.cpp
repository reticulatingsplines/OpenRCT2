/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma warning(disable : 4706) // assignment within conditional expression

#include "SceneryGroupObject.h"

#include "../Context.h"
#include "../core/IStream.hpp"
#include "../core/Json.hpp"
#include "../core/Memory.hpp"
#include "../core/String.hpp"
#include "../drawing/Drawing.h"
#include "../drawing/Image.h"
#include "../entity/Staff.h"
#include "../localisation/Language.h"
#include "ObjectManager.h"
#include "ObjectRepository.h"

#include <unordered_map>

using namespace OpenRCT2;

void SceneryGroupObject::ReadLegacy(IReadObjectContext* context, IStream* stream)
{
    stream->Seek(6, STREAM_SEEK_CURRENT);
    stream->Seek(0x80 * 2, STREAM_SEEK_CURRENT);
    _legacyType.entry_count = stream->ReadValue<uint8_t>();
    stream->Seek(1, STREAM_SEEK_CURRENT); // pad_107;
    _legacyType.priority = stream->ReadValue<uint8_t>();
    stream->Seek(1, STREAM_SEEK_CURRENT); // pad_109;
    _legacyType.entertainer_costumes = stream->ReadValue<uint32_t>();

    GetStringTable().Read(context, stream, ObjectStringID::NAME);
    _items = ReadItems(stream);
    GetImageTable().Read(context, stream);
}

void SceneryGroupObject::Load()
{
    GetStringTable().Sort();
    _legacyType.name = language_allocate_object_string(GetName());
    _legacyType.image = gfx_object_allocate_images(GetImageTable().GetImages(), GetImageTable().GetCount());
    _legacyType.entry_count = 0;
}

void SceneryGroupObject::Unload()
{
    language_free_object_string(_legacyType.name);
    gfx_object_free_images(_legacyType.image, GetImageTable().GetCount());

    _legacyType.name = 0;
    _legacyType.image = 0;
}

void SceneryGroupObject::DrawPreview(rct_drawpixelinfo* dpi, int32_t width, int32_t height) const
{
    auto screenCoords = ScreenCoordsXY{ width / 2, height / 2 };

    const auto imageId = ImageId(_legacyType.image + 1, COLOUR_DARK_GREEN);
    gfx_draw_sprite(dpi, imageId, screenCoords - ScreenCoordsXY{ 15, 14 });
}

static std::optional<uint8_t> GetSceneryType(const ObjectType type)
{
    switch (type)
    {
        case ObjectType::SmallScenery:
            return SCENERY_TYPE_SMALL;
        case ObjectType::LargeScenery:
            return SCENERY_TYPE_LARGE;
        case ObjectType::Walls:
            return SCENERY_TYPE_WALL;
        case ObjectType::Banners:
            return SCENERY_TYPE_BANNER;
        case ObjectType::PathBits:
            return SCENERY_TYPE_PATH_ITEM;
        default:
            return std::nullopt;
    }
}

void SceneryGroupObject::UpdateEntryIndexes()
{
    auto context = GetContext();
    auto& objectRepository = context->GetObjectRepository();
    auto& objectManager = context->GetObjectManager();

    _legacyType.entry_count = 0;
    for (const auto& objectEntry : _items)
    {
        auto ori = objectRepository.FindObject(objectEntry);
        if (ori == nullptr)
            continue;
        if (ori->LoadedObject == nullptr)
            continue;

        auto entryIndex = objectManager.GetLoadedObjectEntryIndex(ori->LoadedObject.get());
        Guard::Assert(entryIndex != OBJECT_ENTRY_INDEX_NULL, GUARD_LINE);

        auto sceneryType = GetSceneryType(ori->Type);
        if (sceneryType.has_value())
        {
            _legacyType.scenery_entries[_legacyType.entry_count] = { sceneryType.value(), entryIndex };
            _legacyType.entry_count++;
        }
    }
}

void SceneryGroupObject::SetRepositoryItem(ObjectRepositoryItem* item) const
{
    item->SceneryGroupInfo.Entries = _items;
}

std::vector<ObjectEntryDescriptor> SceneryGroupObject::ReadItems(IStream* stream)
{
    auto items = std::vector<ObjectEntryDescriptor>();
    while (stream->ReadValue<uint8_t>() != 0xFF)
    {
        stream->Seek(-1, STREAM_SEEK_CURRENT);
        auto entry = stream->ReadValue<rct_object_entry>();
        items.emplace_back(entry);
    }
    return items;
}

void SceneryGroupObject::ReadJson(IReadObjectContext* context, json_t& root)
{
    Guard::Assert(root.is_object(), "SceneryGroupObject::ReadJson expects parameter root to be object");

    auto properties = root["properties"];

    if (properties.is_object())
    {
        _legacyType.priority = Json::GetNumber<uint8_t>(properties["priority"]);
        _legacyType.entertainer_costumes = ReadJsonEntertainerCostumes(properties["entertainerCostumes"]);

        _items = ReadJsonEntries(properties["entries"]);
    }

    PopulateTablesFromJson(context, root);
}

uint32_t SceneryGroupObject::ReadJsonEntertainerCostumes(json_t& jCostumes)
{
    uint32_t costumes = 0;
    for (auto& jCostume : jCostumes)
    {
        auto entertainer = ParseEntertainerCostume(Json::GetString(jCostume));
        auto peepSprite = EntertainerCostumeToSprite(entertainer);
        costumes |= 1 << (static_cast<uint8_t>(peepSprite));
    }
    return costumes;
}

EntertainerCostume SceneryGroupObject::ParseEntertainerCostume(const std::string& s)
{
    if (s == "panda")
        return EntertainerCostume::Panda;
    if (s == "tiger")
        return EntertainerCostume::Tiger;
    if (s == "elephant")
        return EntertainerCostume::Elephant;
    if (s == "roman")
        return EntertainerCostume::Roman;
    if (s == "gorilla")
        return EntertainerCostume::Gorilla;
    if (s == "snowman")
        return EntertainerCostume::Snowman;
    if (s == "knight")
        return EntertainerCostume::Knight;
    if (s == "astronaut")
        return EntertainerCostume::Astronaut;
    if (s == "bandit")
        return EntertainerCostume::Bandit;
    if (s == "sheriff")
        return EntertainerCostume::Sheriff;
    if (s == "pirate")
        return EntertainerCostume::Pirate;
    return EntertainerCostume::Panda;
}

std::vector<ObjectEntryDescriptor> SceneryGroupObject::ReadJsonEntries(json_t& jEntries)
{
    std::vector<ObjectEntryDescriptor> entries;

    for (const auto& jEntry : jEntries)
    {
        entries.emplace_back(Json::GetString(jEntry));
    }
    return entries;
}

uint16_t SceneryGroupObject::GetNumIncludedObjects() const
{
    return static_cast<uint16_t>(_items.size());
}

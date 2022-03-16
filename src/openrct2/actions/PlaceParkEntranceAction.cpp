/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "PlaceParkEntranceAction.h"

#include "../Cheats.h"
#include "../OpenRCT2.h"
#include "../core/MemoryStream.h"
#include "../localisation/StringIds.h"
#include "../management/Finance.h"
#include "../world/ConstructionClearance.h"
#include "../world/Entrance.h"
#include "../world/Footpath.h"
#include "../world/MapAnimation.h"
#include "../world/Park.h"
#include "../world/Surface.h"

PlaceParkEntranceAction::PlaceParkEntranceAction(const CoordsXYZD& location, ObjectEntryIndex pathType)
    : _loc(location)
    , _pathType(pathType)
{
}

uint16_t PlaceParkEntranceAction::GetActionFlags() const
{
    return GameActionBase::GetActionFlags() | GameActions::Flags::EditorOnly;
}

void PlaceParkEntranceAction::Serialise(DataSerialiser& stream)
{
    GameAction::Serialise(stream);

    stream << DS_TAG(_loc);
    stream << DS_TAG(_pathType);
}

GameActions::Result PlaceParkEntranceAction::Query() const
{
    if (!(gScreenFlags & SCREEN_FLAGS_EDITOR) && !gCheatsSandboxMode)
    {
        return GameActions::Result(GameActions::Status::NotInEditorMode, STR_CANT_BUILD_THIS_HERE, STR_NONE);
    }

    auto res = GameActions::Result();
    res.Expenditure = ExpenditureType::LandPurchase;
    res.Position = { _loc.x, _loc.y, _loc.z };

    auto mapSizeUnits = GetMapSizeUnits() - CoordsXY{ COORDS_XY_STEP, COORDS_XY_STEP };
    if (!LocationValid(_loc) || _loc.x <= COORDS_XY_STEP || _loc.y <= COORDS_XY_STEP || _loc.x >= mapSizeUnits.x
        || _loc.y >= mapSizeUnits.y)
    {
        return GameActions::Result(
            GameActions::Status::InvalidParameters, STR_CANT_BUILD_THIS_HERE, STR_TOO_CLOSE_TO_EDGE_OF_MAP);
    }

    if (!CheckMapCapacity(3))
    {
        return GameActions::Result(
            GameActions::Status::NoFreeElements, STR_CANT_BUILD_THIS_HERE, STR_ERR_LANDSCAPE_DATA_AREA_FULL);
    }

    if (gParkEntrances.size() >= OpenRCT2::Limits::MaxParkEntrances)
    {
        return GameActions::Result(
            GameActions::Status::InvalidParameters, STR_CANT_BUILD_THIS_HERE, STR_ERR_TOO_MANY_PARK_ENTRANCES);
    }

    auto zLow = _loc.z;
    auto zHigh = zLow + ParkEntranceHeight;
    CoordsXYZ entranceLoc = _loc;
    for (uint8_t index = 0; index < 3; index++)
    {
        if (index == 1)
        {
            entranceLoc += CoordsDirectionDelta[(_loc.direction - 1) & 0x3];
        }
        else if (index == 2)
        {
            entranceLoc.x += CoordsDirectionDelta[(_loc.direction + 1) & 0x3].x * 2;
            entranceLoc.y += CoordsDirectionDelta[(_loc.direction + 1) & 0x3].y * 2;
        }

        if (auto res2 = MapCanConstructAt({ entranceLoc, zLow, zHigh }, { 0b1111, 0 }); res2.Error != GameActions::Status::Ok)
        {
            res2.ErrorTitle = STR_CANT_BUILD_THIS_HERE;
            return res2;
        }

        // Check that entrance element does not already exist at this location
        EntranceElement* entranceElement = map_get_park_entrance_element_at(entranceLoc, false);
        if (entranceElement != nullptr)
        {
            return GameActions::Result(GameActions::Status::ItemAlreadyPlaced, STR_CANT_BUILD_THIS_HERE, STR_NONE);
        }
    }

    return res;
}

GameActions::Result PlaceParkEntranceAction::Execute() const
{
    auto res = GameActions::Result();
    res.Expenditure = ExpenditureType::LandPurchase;
    res.Position = CoordsXYZ{ _loc.x, _loc.y, _loc.z };

    uint32_t flags = GetFlags();

    gParkEntrances.push_back(_loc);

    auto zLow = _loc.z;
    auto zHigh = zLow + ParkEntranceHeight;
    CoordsXY entranceLoc = { _loc.x, _loc.y };
    for (uint8_t index = 0; index < 3; index++)
    {
        if (index == 1)
        {
            entranceLoc.x += CoordsDirectionDelta[(_loc.direction - 1) & 0x3].x;
            entranceLoc.y += CoordsDirectionDelta[(_loc.direction - 1) & 0x3].y;
        }
        else if (index == 2)
        {
            entranceLoc.x += CoordsDirectionDelta[(_loc.direction + 1) & 0x3].x * 2;
            entranceLoc.y += CoordsDirectionDelta[(_loc.direction + 1) & 0x3].y * 2;
        }

        if (!(flags & GAME_COMMAND_FLAG_GHOST))
        {
            SurfaceElement* surfaceElement = map_get_surface_element_at(entranceLoc);
            if (surfaceElement != nullptr)
            {
                surfaceElement->SetOwnership(OWNERSHIP_UNOWNED);
            }
        }

        auto* entranceElement = TileElementInsert<EntranceElement>(CoordsXYZ{ entranceLoc, zLow }, 0b1111);
        Guard::Assert(entranceElement != nullptr);

        entranceElement->SetClearanceZ(zHigh);
        entranceElement->SetGhost(flags & GAME_COMMAND_FLAG_GHOST);
        entranceElement->SetDirection(_loc.direction);
        entranceElement->SetSequenceIndex(index);
        entranceElement->SetEntranceType(ENTRANCE_TYPE_PARK_ENTRANCE);
        if (gFootpathSelection.LegacyPath == OBJECT_ENTRY_INDEX_NULL)
        {
            entranceElement->SetSurfaceEntryIndex(gFootpathSelection.NormalSurface);
        }
        else
        {
            entranceElement->SetLegacyPathEntryIndex(gFootpathSelection.LegacyPath);
        }

        if (!entranceElement->IsGhost())
        {
            footpath_connect_edges(entranceLoc, entranceElement->as<TileElement>(), GAME_COMMAND_FLAG_APPLY);
        }

        update_park_fences(entranceLoc);
        update_park_fences({ entranceLoc.x - COORDS_XY_STEP, entranceLoc.y });
        update_park_fences({ entranceLoc.x + COORDS_XY_STEP, entranceLoc.y });
        update_park_fences({ entranceLoc.x, entranceLoc.y - COORDS_XY_STEP });
        update_park_fences({ entranceLoc.x, entranceLoc.y + COORDS_XY_STEP });

        map_invalidate_tile({ entranceLoc, entranceElement->GetBaseZ(), entranceElement->GetClearanceZ() });

        if (index == 0)
        {
            map_animation_create(MAP_ANIMATION_TYPE_PARK_ENTRANCE, { entranceLoc, zLow });
        }
    }

    return res;
}

bool PlaceParkEntranceAction::CheckMapCapacity(int16_t numTiles) const
{
    CoordsXYZ entranceLoc = _loc;
    for (uint8_t index = 0; index < 3; index++)
    {
        if (index == 1)
        {
            entranceLoc += CoordsDirectionDelta[(_loc.direction - 1) & 0x3];
        }
        else if (index == 2)
        {
            entranceLoc.x += CoordsDirectionDelta[(_loc.direction + 1) & 0x3].x * 2;
            entranceLoc.y += CoordsDirectionDelta[(_loc.direction + 1) & 0x3].y * 2;
        }
        if (!MapCheckCapacityAndReorganise(entranceLoc, numTiles))
        {
            return false;
        }
    }
    return true;
}

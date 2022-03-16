/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "RideDemolishAction.h"

#include "../Cheats.h"
#include "../Context.h"
#include "../GameState.h"
#include "../core/MemoryStream.h"
#include "../drawing/Drawing.h"
#include "../entity/EntityList.h"
#include "../interface/Window.h"
#include "../localisation/Localisation.h"
#include "../management/NewsItem.h"
#include "../peep/RideUseSystem.h"
#include "../ride/Ride.h"
#include "../ride/RideData.h"
#include "../ui/UiContext.h"
#include "../ui/WindowManager.h"
#include "../world/Banner.h"
#include "../world/Park.h"
#include "../world/TileElementsView.h"
#include "MazeSetTrackAction.h"
#include "TrackRemoveAction.h"

using namespace OpenRCT2;

RideDemolishAction::RideDemolishAction(RideId rideIndex, uint8_t modifyType)
    : _rideIndex(rideIndex)
    , _modifyType(modifyType)
{
}

void RideDemolishAction::AcceptParameters(GameActionParameterVisitor& visitor)
{
    visitor.Visit("ride", _rideIndex);
    visitor.Visit("modifyType", _modifyType);
}

uint32_t RideDemolishAction::GetCooldownTime() const
{
    return 1000;
}

void RideDemolishAction::Serialise(DataSerialiser& stream)
{
    GameAction::Serialise(stream);

    stream << DS_TAG(_rideIndex) << DS_TAG(_modifyType);
}

GameActions::Result RideDemolishAction::Query() const
{
    auto ride = get_ride(_rideIndex);
    if (ride == nullptr)
    {
        log_warning("Invalid game command for ride %u", _rideIndex.ToUnderlying());
        return GameActions::Result(GameActions::Status::InvalidParameters, STR_CANT_DEMOLISH_RIDE, STR_NONE);
    }

    if (ride->lifecycle_flags & (RIDE_LIFECYCLE_INDESTRUCTIBLE | RIDE_LIFECYCLE_INDESTRUCTIBLE_TRACK)
        && _modifyType == RIDE_MODIFY_DEMOLISH)
    {
        return GameActions::Result(
            GameActions::Status::NoClearance, STR_CANT_DEMOLISH_RIDE,
            STR_LOCAL_AUTHORITY_FORBIDS_DEMOLITION_OR_MODIFICATIONS_TO_THIS_RIDE);
    }

    GameActions::Result result = GameActions::Result();

    if (_modifyType == RIDE_MODIFY_RENEW)
    {
        if (ride->status != RideStatus::Closed && ride->status != RideStatus::Simulating)
        {
            return GameActions::Result(GameActions::Status::Disallowed, STR_CANT_REFURBISH_RIDE, STR_MUST_BE_CLOSED_FIRST);
        }

        if (ride->num_riders > 0)
        {
            return GameActions::Result(GameActions::Status::Disallowed, STR_CANT_REFURBISH_RIDE, STR_RIDE_NOT_YET_EMPTY);
        }

        if (!(ride->lifecycle_flags & RIDE_LIFECYCLE_EVER_BEEN_OPENED)
            || ride->GetRideTypeDescriptor().AvailableBreakdowns == 0)
        {
            return GameActions::Result(GameActions::Status::Disallowed, STR_CANT_REFURBISH_RIDE, STR_CANT_REFURBISH_NOT_NEEDED);
        }

        result.ErrorTitle = STR_CANT_REFURBISH_RIDE;
        result.Cost = GetRefurbishPrice(ride);
    }

    return result;
}

GameActions::Result RideDemolishAction::Execute() const
{
    auto ride = get_ride(_rideIndex);
    if (ride == nullptr)
    {
        log_warning("Invalid game command for ride %u", _rideIndex.ToUnderlying());
        return GameActions::Result(GameActions::Status::InvalidParameters, STR_CANT_DEMOLISH_RIDE, STR_NONE);
    }

    switch (_modifyType)
    {
        case RIDE_MODIFY_DEMOLISH:
            return DemolishRide(ride);
        case RIDE_MODIFY_RENEW:
            return RefurbishRide(ride);
    }

    return GameActions::Result(GameActions::Status::InvalidParameters, STR_CANT_DO_THIS, STR_NONE);
}

GameActions::Result RideDemolishAction::DemolishRide(Ride* ride) const
{
    money32 refundPrice = DemolishTracks();

    ride_clear_for_construction(ride);
    ride->RemovePeeps();
    ride->StopGuestsQueuing();

    ride->ValidateStations();
    ride_clear_leftover_entrances(ride);

    const auto rideId = ride->id;
    News::DisableNewsItems(News::ItemType::Ride, rideId.ToUnderlying());

    UnlinkAllBannersForRide(ride->id);

    RideUse::GetHistory().RemoveValue(ride->id);
    for (auto peep : EntityList<Guest>())
    {
        peep->RemoveRideFromMemory(ride->id);
    }

    MarketingCancelCampaignsForRide(_rideIndex);

    auto res = GameActions::Result();
    res.Expenditure = ExpenditureType::RideConstruction;
    res.Cost = refundPrice;

    if (!ride->overall_view.IsNull())
    {
        auto xy = ride->overall_view.ToTileCentre();
        res.Position = { xy, tile_element_height(xy) };
    }

    ride->Delete();
    gParkValue = GetContext()->GetGameState()->GetPark().CalculateParkValue();

    // Close windows related to the demolished ride
    window_close_by_number(WC_RIDE_CONSTRUCTION, rideId.ToUnderlying());
    window_close_by_number(WC_RIDE, rideId.ToUnderlying());
    window_close_by_number(WC_DEMOLISH_RIDE_PROMPT, rideId.ToUnderlying());
    window_close_by_class(WC_NEW_CAMPAIGN);

    // Refresh windows that display the ride name
    auto windowManager = OpenRCT2::GetContext()->GetUiContext()->GetWindowManager();
    windowManager->BroadcastIntent(Intent(INTENT_ACTION_REFRESH_CAMPAIGN_RIDE_LIST));
    windowManager->BroadcastIntent(Intent(INTENT_ACTION_REFRESH_RIDE_LIST));
    windowManager->BroadcastIntent(Intent(INTENT_ACTION_REFRESH_GUEST_LIST));

    scrolling_text_invalidate();
    gfx_invalidate_screen();

    return res;
}

money32 RideDemolishAction::MazeRemoveTrack(const CoordsXYZD& coords) const
{
    auto setMazeTrack = MazeSetTrackAction(coords, false, _rideIndex, GC_SET_MAZE_TRACK_FILL);
    setMazeTrack.SetFlags(GetFlags());

    auto execRes = GameActions::ExecuteNested(&setMazeTrack);
    if (execRes.Error == GameActions::Status::Ok)
    {
        return execRes.Cost;
    }

    return MONEY32_UNDEFINED;
}

money32 RideDemolishAction::DemolishTracks() const
{
    money32 refundPrice = 0;

    uint8_t oldpaused = gGamePaused;
    gGamePaused = 0;

    for (TileCoordsXY tilePos = {}; tilePos.x < gMapSize.x; ++tilePos.x)
    {
        for (tilePos.y = 0; tilePos.y < gMapSize.y; ++tilePos.y)
        {
            const auto tileCoords = tilePos.ToCoordsXY();
            // Keep retrying a tile coordinate until there are no more items to remove
            bool itemRemoved = false;
            do
            {
                itemRemoved = false;
                for (auto* trackElement : TileElementsView<TrackElement>(tileCoords))
                {
                    if (trackElement->GetRideIndex() != _rideIndex)
                        continue;

                    const auto location = CoordsXYZD(tileCoords, trackElement->GetBaseZ(), trackElement->GetDirection());
                    const auto type = trackElement->GetTrackType();

                    if (type != TrackElemType::Maze)
                    {
                        auto trackRemoveAction = TrackRemoveAction(type, trackElement->GetSequenceIndex(), location);
                        trackRemoveAction.SetFlags(GAME_COMMAND_FLAG_NO_SPEND);

                        auto removRes = GameActions::ExecuteNested(&trackRemoveAction);
                        itemRemoved = true;
                        if (removRes.Error != GameActions::Status::Ok)
                        {
                            tile_element_remove(trackElement->as<TileElement>());
                        }
                        else
                        {
                            refundPrice += removRes.Cost;
                        }
                        continue;
                    }

                    static constexpr const CoordsXY DirOffsets[] = {
                        { 0, 0 },
                        { 0, 16 },
                        { 16, 16 },
                        { 16, 0 },
                    };
                    for (Direction dir : ALL_DIRECTIONS)
                    {
                        const CoordsXYZ off = { DirOffsets[dir], 0 };
                        money32 removePrice = MazeRemoveTrack({ location + off, dir });
                        if (removePrice != MONEY32_UNDEFINED)
                        {
                            refundPrice += removePrice;
                            itemRemoved = true;
                        }
                        else
                            break;
                    }
                }
            } while (itemRemoved);
        }
    }

    gGamePaused = oldpaused;
    return refundPrice;
}

GameActions::Result RideDemolishAction::RefurbishRide(Ride* ride) const
{
    auto res = GameActions::Result();
    res.Expenditure = ExpenditureType::RideConstruction;
    res.Cost = GetRefurbishPrice(ride);

    ride->Renew();

    ride->lifecycle_flags &= ~RIDE_LIFECYCLE_EVER_BEEN_OPENED;
    ride->last_crash_type = RIDE_CRASH_TYPE_NONE;

    ride->window_invalidate_flags |= RIDE_INVALIDATE_RIDE_MAINTENANCE | RIDE_INVALIDATE_RIDE_CUSTOMER;

    if (!ride->overall_view.IsNull())
    {
        auto location = ride->overall_view.ToTileCentre();
        res.Position = { location, tile_element_height(location) };
    }

    window_close_by_number(WC_DEMOLISH_RIDE_PROMPT, _rideIndex.ToUnderlying());

    return res;
}

money32 RideDemolishAction::GetRefurbishPrice(const Ride* ride) const
{
    return -GetRefundPrice(ride) / 2;
}

money32 RideDemolishAction::GetRefundPrice(const Ride* ride) const
{
    return ride_get_refund_price(ride);
}

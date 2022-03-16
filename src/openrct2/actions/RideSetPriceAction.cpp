/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "RideSetPriceAction.h"

#include "../Cheats.h"
#include "../common.h"
#include "../core/MemoryStream.h"
#include "../interface/Window.h"
#include "../localisation/Localisation.h"
#include "../localisation/StringIds.h"
#include "../management/Finance.h"
#include "../ride/Ride.h"
#include "../ride/RideData.h"
#include "../ride/ShopItem.h"
#include "../world/Park.h"

RideSetPriceAction::RideSetPriceAction(RideId rideIndex, money16 price, bool primaryPrice)
    : _rideIndex(rideIndex)
    , _price(price)
    , _primaryPrice(primaryPrice)
{
}

void RideSetPriceAction::AcceptParameters(GameActionParameterVisitor& visitor)
{
    visitor.Visit("ride", _rideIndex);
    visitor.Visit("price", _price);
    visitor.Visit("isPrimaryPrice", _primaryPrice);
}

uint16_t RideSetPriceAction::GetActionFlags() const
{
    return GameAction::GetActionFlags() | GameActions::Flags::AllowWhilePaused;
}

void RideSetPriceAction::Serialise(DataSerialiser& stream)
{
    GameAction::Serialise(stream);

    stream << DS_TAG(_rideIndex) << DS_TAG(_price) << DS_TAG(_primaryPrice);
}

GameActions::Result RideSetPriceAction::Query() const
{
    GameActions::Result res = GameActions::Result();

    auto ride = get_ride(_rideIndex);
    if (ride == nullptr)
    {
        log_warning("Invalid game command, ride_id = %u", _rideIndex.ToUnderlying());
        return GameActions::Result(GameActions::Status::InvalidParameters, STR_NONE, STR_NONE);
    }

    rct_ride_entry* rideEntry = get_ride_entry(ride->subtype);
    if (rideEntry == nullptr)
    {
        log_warning("Invalid game command for ride %u", _rideIndex.ToUnderlying());
        return GameActions::Result(GameActions::Status::InvalidParameters, STR_NONE, STR_NONE);
    }

    return res;
}

GameActions::Result RideSetPriceAction::Execute() const
{
    GameActions::Result res = GameActions::Result();
    res.Expenditure = ExpenditureType::ParkRideTickets;

    auto ride = get_ride(_rideIndex);
    if (ride == nullptr)
    {
        log_warning("Invalid game command, ride_id = %u", _rideIndex.ToUnderlying());
        return GameActions::Result(GameActions::Status::InvalidParameters, STR_NONE, STR_NONE);
    }

    rct_ride_entry* rideEntry = get_ride_entry(ride->subtype);
    if (rideEntry == nullptr)
    {
        log_warning("Invalid game command for ride %u", _rideIndex.ToUnderlying());
        return GameActions::Result(GameActions::Status::InvalidParameters, STR_NONE, STR_NONE);
    }

    if (!ride->overall_view.IsNull())
    {
        auto location = ride->overall_view.ToTileCentre();
        res.Position = { location, tile_element_height(location) };
    }

    ShopItem shopItem;
    if (_primaryPrice)
    {
        shopItem = ShopItem::Admission;
        if (ride->type != RIDE_TYPE_TOILETS)
        {
            shopItem = rideEntry->shop_item[0];
            if (shopItem == ShopItem::None)
            {
                ride->price[0] = _price;
                window_invalidate_by_class(WC_RIDE);
                return res;
            }
        }
        // Check same price in park flags
        if (!shop_item_has_common_price(shopItem))
        {
            ride->price[0] = _price;
            window_invalidate_by_class(WC_RIDE);
            return res;
        }
    }
    else
    {
        shopItem = rideEntry->shop_item[1];
        if (shopItem == ShopItem::None)
        {
            shopItem = ride->GetRideTypeDescriptor().PhotoItem;
            if ((ride->lifecycle_flags & RIDE_LIFECYCLE_ON_RIDE_PHOTO) == 0)
            {
                ride->price[1] = _price;
                window_invalidate_by_class(WC_RIDE);
                return res;
            }
        }
        // Check same price in park flags
        if (!shop_item_has_common_price(shopItem))
        {
            ride->price[1] = _price;
            window_invalidate_by_class(WC_RIDE);
            return res;
        }
    }

    // Synchronize prices if enabled.
    RideSetCommonPrice(shopItem);

    return res;
}

void RideSetPriceAction::RideSetCommonPrice(ShopItem shopItem) const
{
    for (auto& ride : GetRideManager())
    {
        auto invalidate = false;
        auto rideEntry = get_ride_entry(ride.subtype);
        if (ride.type == RIDE_TYPE_TOILETS && shopItem == ShopItem::Admission)
        {
            if (ride.price[0] != _price)
            {
                ride.price[0] = _price;
                invalidate = true;
            }
        }
        else if (rideEntry != nullptr && rideEntry->shop_item[0] == shopItem)
        {
            if (ride.price[0] != _price)
            {
                ride.price[0] = _price;
                invalidate = true;
            }
        }
        if (rideEntry != nullptr)
        {
            // If the shop item is the same or an on-ride photo
            if (rideEntry->shop_item[1] == shopItem
                || (rideEntry->shop_item[1] == ShopItem::None && GetShopItemDescriptor(shopItem).IsPhoto()))
            {
                if (ride.price[1] != _price)
                {
                    ride.price[1] = _price;
                    invalidate = true;
                }
            }
        }
        if (invalidate)
        {
            window_invalidate_by_number(WC_RIDE, ride.id.ToUnderlying());
        }
    }
}

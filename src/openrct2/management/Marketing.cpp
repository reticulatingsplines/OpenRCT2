/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "Marketing.h"

#include "../Cheats.h"
#include "../Game.h"
#include "../config/Config.h"
#include "../entity/Guest.h"
#include "../interface/Window.h"
#include "../localisation/Localisation.h"
#include "../ride/Ride.h"
#include "../ride/RideData.h"
#include "../ride/ShopItem.h"
#include "../world/Park.h"
#include "Finance.h"
#include "NewsItem.h"

const money16 AdvertisingCampaignPricePerWeek[] = {
    MONEY(50, 00),  // PARK_ENTRY_FREE
    MONEY(50, 00),  // RIDE_FREE
    MONEY(50, 00),  // PARK_ENTRY_HALF_PRICE
    MONEY(50, 00),  // FOOD_OR_DRINK_FREE
    MONEY(350, 00), // PARK
    MONEY(200, 00), // RIDE
};

static constexpr const uint16_t AdvertisingCampaignGuestGenerationProbabilities[] = {
    400, 300, 200, 200, 250, 200,
};

std::vector<MarketingCampaign> gMarketingCampaigns;

uint16_t marketing_get_campaign_guest_generation_probability(int32_t campaignType)
{
    auto campaign = marketing_get_campaign(campaignType);
    if (campaign == nullptr)
        return 0;

    // Lower probability of guest generation if price was already low
    auto probability = AdvertisingCampaignGuestGenerationProbabilities[campaign->Type];
    switch (campaign->Type)
    {
        case ADVERTISING_CAMPAIGN_PARK_ENTRY_FREE:
            if (park_get_entrance_fee() < MONEY(4, 00))
                probability /= 8;
            break;
        case ADVERTISING_CAMPAIGN_PARK_ENTRY_HALF_PRICE:
            if (park_get_entrance_fee() < MONEY(6, 00))
                probability /= 8;
            break;
        case ADVERTISING_CAMPAIGN_RIDE_FREE:
        {
            auto ride = get_ride(campaign->RideId);
            if (ride == nullptr || ride->price[0] < MONEY(0, 30))
                probability /= 8;
            break;
        }
    }

    return probability;
}

static void marketing_raise_finished_notification(const MarketingCampaign& campaign)
{
    if (gConfigNotifications.park_marketing_campaign_finished)
    {
        Formatter ft;
        // This sets the string parameters for the marketing types that have an argument.
        if (campaign.Type == ADVERTISING_CAMPAIGN_RIDE_FREE || campaign.Type == ADVERTISING_CAMPAIGN_RIDE)
        {
            auto ride = get_ride(campaign.RideId);
            if (ride != nullptr)
            {
                ride->FormatNameTo(ft);
            }
        }
        else if (campaign.Type == ADVERTISING_CAMPAIGN_FOOD_OR_DRINK_FREE)
        {
            ft.Add<rct_string_id>(GetShopItemDescriptor(campaign.ShopItemType).Naming.Plural);
        }

        News::AddItemToQueue(News::ItemType::Money, MarketingCampaignNames[campaign.Type][2], 0, ft);
    }
}

/**
 * Update status of marketing campaigns and send produce a news item when they have finished.
 *  rct2: 0x0069E0C1
 */
void marketing_update()
{
    if (gCheatsNeverendingMarketing)
        return;

    for (auto it = gMarketingCampaigns.begin(); it != gMarketingCampaigns.end();)
    {
        auto& campaign = *it;
        if (campaign.Flags & MarketingCampaignFlags::FIRST_WEEK)
        {
            // This ensures the campaign is active for x full weeks if started within the
            // middle of a week.
            campaign.Flags &= ~MarketingCampaignFlags::FIRST_WEEK;
        }
        else if (campaign.WeeksLeft > 0)
        {
            campaign.WeeksLeft--;
        }

        if (campaign.WeeksLeft == 0)
        {
            marketing_raise_finished_notification(campaign);
            it = gMarketingCampaigns.erase(it);
        }
        else
        {
            ++it;
        }
    }

    window_invalidate_by_class(WC_FINANCES);
}

void marketing_set_guest_campaign(Guest* peep, int32_t campaignType)
{
    auto campaign = marketing_get_campaign(campaignType);
    if (campaign == nullptr)
        return;

    switch (campaign->Type)
    {
        case ADVERTISING_CAMPAIGN_PARK_ENTRY_FREE:
            peep->GiveItem(ShopItem::Voucher);
            peep->VoucherType = VOUCHER_TYPE_PARK_ENTRY_FREE;
            break;
        case ADVERTISING_CAMPAIGN_RIDE_FREE:
            peep->GiveItem(ShopItem::Voucher);
            peep->VoucherType = VOUCHER_TYPE_RIDE_FREE;
            peep->VoucherRideId = campaign->RideId;
            peep->GuestHeadingToRideId = campaign->RideId;
            peep->GuestIsLostCountdown = 240;
            break;
        case ADVERTISING_CAMPAIGN_PARK_ENTRY_HALF_PRICE:
            peep->GiveItem(ShopItem::Voucher);
            peep->VoucherType = VOUCHER_TYPE_PARK_ENTRY_HALF_PRICE;
            break;
        case ADVERTISING_CAMPAIGN_FOOD_OR_DRINK_FREE:
            peep->GiveItem(ShopItem::Voucher);
            peep->VoucherType = VOUCHER_TYPE_FOOD_OR_DRINK_FREE;
            peep->VoucherShopItem = campaign->ShopItemType;
            break;
        case ADVERTISING_CAMPAIGN_PARK:
            break;
        case ADVERTISING_CAMPAIGN_RIDE:
            peep->GuestHeadingToRideId = campaign->RideId;
            peep->GuestIsLostCountdown = 240;
            break;
    }
}

bool marketing_is_campaign_type_applicable(int32_t campaignType)
{
    switch (campaignType)
    {
        case ADVERTISING_CAMPAIGN_PARK_ENTRY_FREE:
        case ADVERTISING_CAMPAIGN_PARK_ENTRY_HALF_PRICE:
            if (!park_entry_price_unlocked())
                return false;
            return true;

        case ADVERTISING_CAMPAIGN_RIDE_FREE:
            if (!park_ride_prices_unlocked())
                return false;

            // fall-through
        case ADVERTISING_CAMPAIGN_RIDE:
            // Check if any rides exist
            for (auto& ride : GetRideManager())
            {
                if (ride.IsRide())
                {
                    return true;
                }
            }
            return false;

        case ADVERTISING_CAMPAIGN_FOOD_OR_DRINK_FREE:
            // Check if any food or drink stalls exist
            for (auto& ride : GetRideManager())
            {
                auto rideEntry = ride.GetRideEntry();
                if (rideEntry != nullptr)
                {
                    for (auto& item : rideEntry->shop_item)
                    {
                        if (GetShopItemDescriptor(item).IsFoodOrDrink())
                        {
                            return true;
                        }
                    }
                }
            }
            return false;

        default:
            return true;
    }
}

MarketingCampaign* marketing_get_campaign(int32_t campaignType)
{
    for (auto& campaign : gMarketingCampaigns)
    {
        if (campaign.Type == campaignType)
        {
            return &campaign;
        }
    }
    return nullptr;
}

void marketing_new_campaign(const MarketingCampaign& campaign)
{
    // Do not allow same campaign twice, just overwrite
    auto currentCampaign = marketing_get_campaign(campaign.Type);
    if (currentCampaign != nullptr)
    {
        *currentCampaign = campaign;
    }
    else
    {
        gMarketingCampaigns.push_back(campaign);
    }
}

void MarketingCancelCampaignsForRide(const ride_id_t rideId)
{
    auto isCampaignForRideFn = [&rideId](MarketingCampaign& campaign) {
        if (campaign.Type == ADVERTISING_CAMPAIGN_RIDE_FREE || campaign.Type == ADVERTISING_CAMPAIGN_RIDE)
        {
            return campaign.RideId == rideId;
        }
        return false;
    };

    auto& v = gMarketingCampaigns;
    auto removedIt = std::remove_if(v.begin(), v.end(), isCampaignForRideFn);
    v.erase(removedIt, v.end());
}

/*****************************************************************************
 * Copyright (c) 2014-2021 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.

GuestHeadingToRideId
 *****************************************************************************/

#pragma once

#ifdef ENABLE_SCRIPTING

#    include "ScPeep.hpp"
#    include "../ride/ScRide.hpp"


namespace OpenRCT2::Scripting
{
    class ScGuest : public ScPeep
    {
    public:
        ScGuest(uint16_t id);

        static void Register(duk_context* ctx);


    private:
        Guest* GetGuest() const;

        uint8_t tshirtColour_get() const;
        void tshirtColour_set(uint8_t value);

        uint8_t trousersColour_get() const;
        void trousersColour_set(uint8_t value);

        uint8_t balloonColour_get() const;
        void balloonColour_set(uint8_t value);

        uint8_t hatColour_get() const;
        void hatColour_set(uint8_t value);

        uint8_t umbrellaColour_get() const;
        void umbrellaColour_set(uint8_t value);

        uint8_t happiness_get() const;
        void happiness_set(uint8_t value);

        uint8_t happinessTarget_get() const;
        void happinessTarget_set(uint8_t value);

        uint8_t nausea_get() const;
        void nausea_set(uint8_t value);

        uint8_t nauseaTarget_get() const;
        void nauseaTarget_set(uint8_t value);

        uint8_t hunger_get() const;
        void hunger_set(uint8_t value);

        uint8_t thirst_get() const;
        void thirst_set(uint8_t value);

        uint8_t toilet_get() const;
        void toilet_set(uint8_t value);

        uint8_t mass_get() const;
        void mass_set(uint8_t value);

        uint8_t minIntensity_get() const;
        void minIntensity_set(uint8_t value);

        uint8_t maxIntensity_get() const;
        void maxIntensity_set(uint8_t value);

        uint8_t nauseaTolerance_get() const;
        void nauseaTolerance_set(uint8_t value);

        int32_t cash_get() const;
        void cash_set(int32_t value);

        bool isInPark_get() const;

        bool isLost_get() const;

        uint8_t lostCountdown_get() const;
        void lostCountdown_set(uint8_t value);

        int32_t headingToRideId_get() const;
        void headingToRideId_set(int32_t value);

        void removeAllItems();

        std::vector<DukValue> inventory_get() const;

        void giveItem(int32_t value);

        void removeItem(int32_t value);

        uint8_t voucherType_get() const;
        void voucherType_set(uint8_t value);

        uint8_t voucherId_get() const;
        void voucherId_set(uint8_t value);

        int32_t previousRide_get() const;

        int32_t currentRide_get() const;

        uint8_t currentRideStation_get() const;

        int32_t interactionRide_get() const;

        uint8_t peepState_get() const;

    };

} // namespace OpenRCT2::Scripting

#endif

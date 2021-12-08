/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include "GameAction.h"

enum class RideSetVehicleType : uint8_t
{
    NumTrains,
    NumCarsPerTrain,
    RideEntry,
    Count,
};

class RideSetVehicleAction final : public GameActionBase<GameCommand::SetRideVehicles>
{
private:
    NetworkRideId_t _rideIndex{ RIDE_ID_NULL };
    RideSetVehicleType _type{};
    uint8_t _value{};
    uint8_t _colour{};

public:
    RideSetVehicleAction() = default;
    RideSetVehicleAction(ride_id_t rideIndex, RideSetVehicleType type, uint8_t value, uint8_t colour = 0);

    void AcceptParameters(GameActionParameterVisitor& visitor) override;

    uint16_t GetActionFlags() const override;

    void Serialise(DataSerialiser& stream) override;
    GameActions::Result Query() const override;
    GameActions::Result Execute() const override;

private:
    bool ride_is_vehicle_type_valid(Ride* ride) const;
};

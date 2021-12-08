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

class RideEntranceExitRemoveAction final : public GameActionBase<GameCommand::RemoveRideEntranceOrExit>
{
private:
    CoordsXY _loc;
    NetworkRideId_t _rideIndex{ RIDE_ID_NULL };
    StationIndex _stationNum{ STATION_INDEX_NULL };
    bool _isExit{};

public:
    RideEntranceExitRemoveAction() = default;
    RideEntranceExitRemoveAction(const CoordsXY& loc, ride_id_t rideIndex, StationIndex stationNum, bool isExit);

    void AcceptParameters(GameActionParameterVisitor& visitor) override;

    uint16_t GetActionFlags() const override;

    void Serialise(DataSerialiser& stream) override;
    GameActions::Result Query() const override;
    GameActions::Result Execute() const override;
};

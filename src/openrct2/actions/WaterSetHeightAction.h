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

class WaterSetHeightAction final : public GameActionBase<GameCommand::SetWaterHeight>
{
private:
    CoordsXY _coords;
    uint8_t _height{};

public:
    WaterSetHeightAction() = default;
    WaterSetHeightAction(const CoordsXY& coords, uint8_t height);

    uint16_t GetActionFlags() const override;

    void Serialise(DataSerialiser& stream) override;
    GameActions::Result Query() const override;
    GameActions::Result Execute() const override;

private:
    rct_string_id CheckParameters() const;
};

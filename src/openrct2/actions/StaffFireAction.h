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

class StaffFireAction final : public GameActionBase<GameCommand::FireStaffMember>
{
private:
    uint16_t _spriteId{ SPRITE_INDEX_NULL };

public:
    StaffFireAction() = default;
    StaffFireAction(uint16_t spriteId);

    uint16_t GetActionFlags() const override;

    void Serialise(DataSerialiser& stream) override;
    GameActions::Result Query() const override;
    GameActions::Result Execute() const override;
};

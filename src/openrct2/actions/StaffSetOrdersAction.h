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

class StaffSetOrdersAction final : public GameActionBase<GameCommand::SetStaffOrders>
{
private:
    uint16_t _spriteIndex{ SPRITE_INDEX_NULL };
    uint8_t _ordersId{};

public:
    StaffSetOrdersAction() = default;
    StaffSetOrdersAction(uint16_t spriteIndex, uint8_t ordersId);

    uint16_t GetActionFlags() const override;

    void Serialise(DataSerialiser& stream) override;
    GameActions::Result Query() const override;
    GameActions::Result Execute() const override;
};

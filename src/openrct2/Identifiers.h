/*****************************************************************************
 * Copyright (c) 2014-2021 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include "common.h"
#include "core/Identifier.hpp"

#include <limits>

using ParkEntranceIndex = TIdentifier<uint8_t, std::numeric_limits<uint8_t>::max(), struct ParkEntranceIndexTag>;

using BannerIndex = TIdentifier<uint16_t, std::numeric_limits<uint16_t>::max(), struct BannerIndexTag>;

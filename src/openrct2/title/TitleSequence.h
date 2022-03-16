/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include "../common.h"
#include "../localisation/Localisation.h"
#include "../openrct2/core/IStream.hpp"

#include <memory>

#define TITLE_COMMAND_SCENARIO_LENGTH 64
enum class TitleScript : uint8_t;
struct TitleCommand
{
    TitleScript Type;
    union
    {
        uint8_t SaveIndex; // LOAD (this index is internal only)
        struct             // LOCATION
        {
            uint8_t X;
            uint8_t Y;
        } Location;
        uint8_t Rotations; // ROTATE (counter-clockwise)
        uint8_t Zoom;      // ZOOM
        struct             // FOLLOW
        {
            EntityId SpriteIndex;
            utf8 SpriteName[USER_STRING_MAX_LENGTH];
        } Follow;
        uint8_t Speed;                                // SPEED
        uint16_t Milliseconds;                        // WAIT
        utf8 Scenario[TITLE_COMMAND_SCENARIO_LENGTH]; // LOADSC
    };
};

struct TitleSequence
{
    std::string Name;
    std::string Path;

    std::vector<TitleCommand> Commands;
    std::vector<std::string> Saves;

    bool IsZip = false;
};

struct TitleSequenceParkHandle
{
    std::string HintPath;
    std::unique_ptr<OpenRCT2::IStream> Stream;
};

enum class TitleScript : uint8_t
{
    Undefined = 0xFF,
    Wait = 0,
    Location,
    Rotate,
    Zoom,
    Follow,
    Restart,
    Load,
    End,
    Speed,
    Loop,
    EndLoop,
    LoadSc,
};

constexpr const utf8* TITLE_SEQUENCE_EXTENSION = ".parkseq";
constexpr uint8_t SAVE_INDEX_INVALID = UINT8_MAX;

[[nodiscard]] std::unique_ptr<TitleSequence> CreateTitleSequence();
[[nodiscard]] std::unique_ptr<TitleSequence> LoadTitleSequence(const std::string& path);
[[nodiscard]] std::unique_ptr<TitleSequenceParkHandle> TitleSequenceGetParkHandle(const TitleSequence& seq, size_t index);

bool TitleSequenceSave(const TitleSequence& seq);
bool TitleSequenceAddPark(TitleSequence& seq, const utf8* path, const utf8* name);
bool TitleSequenceRenamePark(TitleSequence& seq, size_t index, const utf8* name);
bool TitleSequenceRemovePark(TitleSequence& seq, size_t index);

bool TitleSequenceIsLoadCommand(const TitleCommand& command);

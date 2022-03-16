/*****************************************************************************
 * Copyright (c) 2014-2021 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#ifdef ENABLE_SCRIPTING

#    include <openrct2/Context.h>
#    include <openrct2/Game.h>
#    include <openrct2/OpenRCT2.h>
#    include <openrct2/ParkImporter.h>
#    include <openrct2/core/String.hpp>
#    include <openrct2/entity/EntityRegistry.h>
#    include <openrct2/object/ObjectManager.h>
#    include <openrct2/scenario/Scenario.h>
#    include <openrct2/scripting/ScriptEngine.h>
#    include <openrct2/title/TitleScreen.h>
#    include <openrct2/title/TitleSequence.h>
#    include <openrct2/title/TitleSequenceManager.h>
#    include <openrct2/title/TitleSequencePlayer.h>

namespace OpenRCT2::Scripting
{
    static const DukEnumMap<TitleScript> TitleScriptMap({
        { "load", TitleScript::Load },
        { "location", TitleScript::Location },
        { "rotate", TitleScript::Rotate },
        { "zoom", TitleScript::Zoom },
        { "follow", TitleScript::Follow },
        { "speed", TitleScript::Speed },
        { "wait", TitleScript::Wait },
        { "loadsc", TitleScript::LoadSc },
        { "restart", TitleScript::Restart },
        { "end", TitleScript::End },
    });

    template<> DukValue ToDuk(duk_context* ctx, const TitleScript& value)
    {
        return ToDuk(ctx, TitleScriptMap[value]);
    }

    template<> DukValue ToDuk(duk_context* ctx, const TitleCommand& value)
    {
        DukObject obj(ctx);
        obj.Set("type", ToDuk(ctx, value.Type));
        switch (value.Type)
        {
            case TitleScript::Load:
                obj.Set("index", value.SaveIndex);
                break;
            case TitleScript::Location:
                obj.Set("x", value.Location.X);
                obj.Set("y", value.Location.Y);
                break;
            case TitleScript::Rotate:
                obj.Set("rotations", value.Rotations);
                break;
            case TitleScript::Zoom:
                obj.Set("zoom", value.Zoom);
                break;
            case TitleScript::Follow:
                if (value.Follow.SpriteIndex.IsNull())
                    obj.Set("id", nullptr);
                else
                    obj.Set("id", value.Follow.SpriteIndex.ToUnderlying());
                break;
            case TitleScript::Speed:
                obj.Set("speed", value.Speed);
                break;
            case TitleScript::Wait:
                obj.Set("duration", value.Milliseconds);
                break;
            case TitleScript::LoadSc:
                obj.Set("scenario", String::ToStringView(value.Scenario, sizeof(value.Scenario)));
                break;
            default:
                break;
        }
        return obj.Take();
    }

    template<> TitleScript FromDuk(const DukValue& value)
    {
        if (value.type() == DukValue::Type::STRING)
            return TitleScriptMap[value.as_string()];
        throw DukException() << "Invalid title command id";
    }

    template<> TitleCommand FromDuk(const DukValue& value)
    {
        auto type = FromDuk<TitleScript>(value["type"]);
        TitleCommand command{};
        command.Type = type;
        switch (type)
        {
            case TitleScript::Load:
                command.SaveIndex = value["index"].as_int();
                break;
            case TitleScript::Location:
                command.Location.X = value["x"].as_int();
                command.Location.Y = value["y"].as_int();
                break;
            case TitleScript::Rotate:
                command.Rotations = value["rotations"].as_int();
                break;
            case TitleScript::Zoom:
                command.Zoom = value["zoom"].as_int();
                break;
            case TitleScript::Follow:
            {
                auto dukId = value["id"];
                if (dukId.type() == DukValue::Type::NUMBER)
                {
                    command.Follow.SpriteIndex = EntityId::FromUnderlying(dukId.as_int());
                }
                else
                {
                    command.Follow.SpriteIndex = EntityId::GetNull();
                }
                break;
            }
            case TitleScript::Speed:
                command.Speed = value["speed"].as_int();
                break;
            case TitleScript::Wait:
                command.Milliseconds = value["duration"].as_int();
                break;
            case TitleScript::LoadSc:
                String::Set(command.Scenario, sizeof(command.Scenario), value["scenario"].as_c_string());
                break;
            default:
                break;
        }
        return command;
    }

    class ScTitleSequencePark
    {
    private:
        std::string _titleSequencePath;
        std::string _fileName;

    public:
        ScTitleSequencePark(std::string_view path, std::string_view fileName)
            : _titleSequencePath(path)
            , _fileName(fileName)
        {
        }

    private:
        std::string fileName_get() const
        {
            return _fileName;
        }

        void fileName_set(const std::string& value)
        {
            if (value == _fileName)
                return;

            auto seq = LoadTitleSequence(_titleSequencePath);
            if (seq != nullptr)
            {
                // Check if name already in use
                auto index = GetIndex(*seq, value);
                if (!index)
                {
                    index = GetIndex(*seq, _fileName);
                    if (index)
                    {
                        TitleSequenceRenamePark(*seq, *index, value.c_str());
                        TitleSequenceSave(*seq);
                    }
                }
            }
        }

        void delete_()
        {
            auto seq = LoadTitleSequence(_titleSequencePath);
            if (seq != nullptr)
            {
                auto index = GetIndex(*seq, _fileName);
                if (index)
                {
                    TitleSequenceRemovePark(*seq, *index);
                    TitleSequenceSave(*seq);
                }
            }
        }

        void load()
        {
            auto seq = LoadTitleSequence(_titleSequencePath);
            if (seq != nullptr)
            {
                auto index = GetIndex(*seq, _fileName);
                if (index)
                {
                    auto handle = TitleSequenceGetParkHandle(*seq, *index);
                    auto isScenario = ParkImporter::ExtensionIsScenario(handle->HintPath);
                    try
                    {
                        auto& objectMgr = GetContext()->GetObjectManager();
                        auto parkImporter = ParkImporter::Create(handle->HintPath);
                        auto result = parkImporter->LoadFromStream(handle->Stream.get(), isScenario);
                        objectMgr.LoadObjects(result.RequiredObjects);
                        parkImporter->Import();

                        auto old = gLoadKeepWindowsOpen;
                        gLoadKeepWindowsOpen = true;
                        if (isScenario)
                            scenario_begin();
                        else
                            game_load_init();
                        gLoadKeepWindowsOpen = old;
                    }
                    catch (const std::exception&)
                    {
                        auto ctx = GetContext()->GetScriptEngine().GetContext();
                        duk_error(ctx, DUK_ERR_ERROR, "Unable to load park.");
                    }
                }
            }
        }

    public:
        static void Register(duk_context* ctx)
        {
            dukglue_register_property(ctx, &ScTitleSequencePark::fileName_get, &ScTitleSequencePark::fileName_set, "fileName");
            dukglue_register_method(ctx, &ScTitleSequencePark::delete_, "delete");
            dukglue_register_method(ctx, &ScTitleSequencePark::load, "load");
        }

    private:
        static std::optional<size_t> GetIndex(const TitleSequence& seq, const std::string_view needle)
        {
            for (size_t i = 0; i < seq.Saves.size(); i++)
            {
                if (seq.Saves[i] == needle)
                {
                    return i;
                }
            }
            return std::nullopt;
        }
    };

    class ScTitleSequence
    {
    private:
        std::string _path;

    public:
        ScTitleSequence(const std::string& path)
        {
            _path = path;
        }

    private:
        std::string name_get() const
        {
            const auto* item = GetItem();
            if (item != nullptr)
            {
                return item->Name;
            }
            return {};
        }

        void name_set(const std::string& value)
        {
            auto index = GetManagerIndex();
            if (index)
            {
                auto newIndex = TitleSequenceManager::RenameItem(*index, value.c_str());

                // Update path to new value
                auto newItem = TitleSequenceManager::GetItem(newIndex);
                _path = newItem != nullptr ? newItem->Path : std::string();
            }
        }

        std::string path_get() const
        {
            const auto* item = GetItem();
            if (item != nullptr)
            {
                return item->Path;
            }
            return {};
        }

        bool isDirectory_get() const
        {
            const auto* item = GetItem();
            if (item != nullptr)
            {
                return !item->IsZip;
            }
            return {};
        }

        bool isReadOnly_get() const
        {
            const auto* item = GetItem();
            if (item != nullptr)
            {
                return item->PredefinedIndex != std::numeric_limits<size_t>::max();
            }
            return {};
        }

        std::vector<std::shared_ptr<ScTitleSequencePark>> parks_get() const
        {
            std::vector<std::shared_ptr<ScTitleSequencePark>> result;
            auto titleSeq = LoadTitleSequence(_path);
            if (titleSeq != nullptr)
            {
                for (size_t i = 0; i < titleSeq->Saves.size(); i++)
                {
                    result.push_back(std::make_shared<ScTitleSequencePark>(_path, titleSeq->Saves[i]));
                }
            }
            return result;
        }

        std::vector<DukValue> commands_get() const
        {
            auto& scriptEngine = GetContext()->GetScriptEngine();
            auto ctx = scriptEngine.GetContext();

            std::vector<DukValue> result;
            auto titleSeq = LoadTitleSequence(_path);
            if (titleSeq != nullptr)
            {
                for (const auto& command : titleSeq->Commands)
                {
                    result.push_back(ToDuk(ctx, command));
                }
            }
            return result;
        }

        void commands_set(const std::vector<DukValue>& value)
        {
            std::vector<TitleCommand> commands;
            for (const auto& v : value)
            {
                auto command = FromDuk<TitleCommand>(v);
                commands.push_back(std::move(command));
            }

            auto titleSeq = LoadTitleSequence(_path);
            titleSeq->Commands = commands;
            TitleSequenceSave(*titleSeq);
        }

        void addPark(const std::string& path, const std::string& fileName)
        {
            auto titleSeq = LoadTitleSequence(_path);
            TitleSequenceAddPark(*titleSeq, path.c_str(), fileName.c_str());
            TitleSequenceSave(*titleSeq);
        }

        std::shared_ptr<ScTitleSequence> clone(const std::string& name) const
        {
            auto copyIndex = GetManagerIndex();
            if (copyIndex)
            {
                auto index = TitleSequenceManager::DuplicateItem(*copyIndex, name.c_str());
                auto* item = TitleSequenceManager::GetItem(index);
                if (item != nullptr)
                {
                    return std::make_shared<ScTitleSequence>(item->Path);
                }
            }
            return nullptr;
        }

        void delete_()
        {
            auto index = GetManagerIndex();
            if (index)
            {
                TitleSequenceManager::DeleteItem(*index);
            }
            _path = {};
        }

        bool isPlaying_get() const
        {
            auto index = GetManagerIndex();
            return index && title_is_previewing_sequence() && *index == title_get_current_sequence();
        }

        DukValue position_get() const
        {
            auto ctx = GetContext()->GetScriptEngine().GetContext();
            if (isPlaying_get())
            {
                auto* player = static_cast<ITitleSequencePlayer*>(title_get_sequence_player());
                if (player != nullptr)
                {
                    return ToDuk(ctx, player->GetCurrentPosition());
                }
            }
            return ToDuk(ctx, nullptr);
        }

        void play()
        {
            auto ctx = GetContext()->GetScriptEngine().GetContext();
            auto index = GetManagerIndex();
            if (index && (!title_is_previewing_sequence() || *index != title_get_current_sequence()))
            {
                if (!title_preview_sequence(*index))
                {
                    duk_error(ctx, DUK_ERR_ERROR, "Failed to load title sequence");
                }
                else if (!(gScreenFlags & SCREEN_FLAGS_TITLE_DEMO))
                {
                    gPreviewingTitleSequenceInGame = true;
                }
            }
        }

        void seek(int32_t position)
        {
            auto ctx = GetContext()->GetScriptEngine().GetContext();
            if (isPlaying_get())
            {
                auto* player = static_cast<ITitleSequencePlayer*>(title_get_sequence_player());
                try
                {
                    player->Seek(position);
                    player->Update();
                }
                catch (...)
                {
                    duk_error(ctx, DUK_ERR_ERROR, "Failed to seek");
                }
            }
        }

        void stop()
        {
            if (isPlaying_get())
            {
                title_stop_previewing_sequence();
            }
        }

    public:
        static void Register(duk_context* ctx)
        {
            dukglue_register_property(ctx, &ScTitleSequence::name_get, &ScTitleSequence::name_set, "name");
            dukglue_register_property(ctx, &ScTitleSequence::path_get, nullptr, "path");
            dukglue_register_property(ctx, &ScTitleSequence::isDirectory_get, nullptr, "isDirectory");
            dukglue_register_property(ctx, &ScTitleSequence::isReadOnly_get, nullptr, "isReadOnly");
            dukglue_register_property(ctx, &ScTitleSequence::parks_get, nullptr, "parks");
            dukglue_register_property(ctx, &ScTitleSequence::commands_get, &ScTitleSequence::commands_set, "commands");
            dukglue_register_property(ctx, &ScTitleSequence::isPlaying_get, nullptr, "isPlaying");
            dukglue_register_property(ctx, &ScTitleSequence::position_get, nullptr, "position");
            dukglue_register_method(ctx, &ScTitleSequence::addPark, "addPark");
            dukglue_register_method(ctx, &ScTitleSequence::clone, "clone");
            dukglue_register_method(ctx, &ScTitleSequence::delete_, "delete");

            dukglue_register_method(ctx, &ScTitleSequence::play, "play");
            dukglue_register_method(ctx, &ScTitleSequence::seek, "seek");
            dukglue_register_method(ctx, &ScTitleSequence::stop, "stop");
        }

    private:
        std::optional<size_t> GetManagerIndex() const
        {
            auto count = TitleSequenceManager::GetCount();
            for (size_t i = 0; i < count; i++)
            {
                auto item = TitleSequenceManager::GetItem(i);
                if (item != nullptr && item->Path == _path)
                {
                    return i;
                }
            }
            return std::nullopt;
        }

        const TitleSequenceManagerItem* GetItem() const
        {
            auto index = GetManagerIndex();
            if (index)
            {
                return TitleSequenceManager::GetItem(*index);
            }
            return nullptr;
        }
    };

    class ScTitleSequenceManager
    {
    private:
        std::vector<std::shared_ptr<ScTitleSequence>> titleSequences_get() const
        {
            std::vector<std::shared_ptr<ScTitleSequence>> result;
            auto count = TitleSequenceManager::GetCount();
            for (size_t i = 0; i < count; i++)
            {
                const auto& path = TitleSequenceManager::GetItem(i)->Path;
                result.push_back(std::make_shared<ScTitleSequence>(path));
            }
            return result;
        }

        std::shared_ptr<ScTitleSequence> create(const std::string& name)
        {
            auto index = TitleSequenceManager::CreateItem(name.c_str());
            auto* item = TitleSequenceManager::GetItem(index);
            if (item != nullptr)
            {
                return std::make_shared<ScTitleSequence>(item->Path);
            }
            return nullptr;
        }

    public:
        static void Register(duk_context* ctx)
        {
            dukglue_register_property(ctx, &ScTitleSequenceManager::titleSequences_get, nullptr, "titleSequences");
            dukglue_register_method(ctx, &ScTitleSequenceManager::create, "create");
        }
    };
} // namespace OpenRCT2::Scripting

#endif

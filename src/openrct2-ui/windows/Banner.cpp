/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include <openrct2-ui/interface/Dropdown.h>
#include <openrct2-ui/interface/Viewport.h>
#include <openrct2-ui/interface/Widget.h>
#include <openrct2-ui/windows/Window.h>
#include <openrct2/Game.h>
#include <openrct2/actions/BannerRemoveAction.h>
#include <openrct2/actions/BannerSetNameAction.h>
#include <openrct2/actions/BannerSetStyleAction.h>
#include <openrct2/config/Config.h>
#include <openrct2/localisation/Localisation.h>
#include <openrct2/sprites.h>
#include <openrct2/world/Banner.h>
#include <openrct2/world/Scenery.h>

static constexpr const int32_t WW = 113;
static constexpr const int32_t WH = 96;
static constexpr const rct_string_id WINDOW_TITLE = STR_BANNER_WINDOW_TITLE;

// clang-format off
enum WindowBannerWidgetIdx {
    WIDX_BACKGROUND,
    WIDX_TITLE,
    WIDX_CLOSE,
    WIDX_VIEWPORT,
    WIDX_BANNER_TEXT,
    WIDX_BANNER_NO_ENTRY,
    WIDX_BANNER_DEMOLISH,
    WIDX_MAIN_COLOUR,
    WIDX_TEXT_COLOUR_DROPDOWN,
    WIDX_TEXT_COLOUR_DROPDOWN_BUTTON
};

static constexpr const rct_string_id BannerColouredTextFormats[] = {
    STR_TEXT_COLOR_BLACK,
    STR_TEXT_COLOR_GREY,
    STR_TEXT_COLOR_WHITE,
    STR_TEXT_COLOR_RED,
    STR_TEXT_COLOR_GREEN,
    STR_TEXT_COLOR_YELLOW,
    STR_TEXT_COLOR_TOPAZ,
    STR_TEXT_COLOR_CELADON,
    STR_TEXT_COLOR_BABYBLUE,
    STR_TEXT_COLOR_PALELAVENDER,
    STR_TEXT_COLOR_PALEGOLD,
    STR_TEXT_COLOR_LIGHTPINK,
    STR_TEXT_COLOR_PEARLAQUA,
    STR_TEXT_COLOR_PALESILVER,
};

static rct_widget window_banner_widgets[] = {
    WINDOW_SHIM(WINDOW_TITLE, WW, WH),
    MakeWidget({      3,      17}, {85, 60}, WindowWidgetType::Viewport,  WindowColour::Secondary, 0x0FFFFFFFE                                        ), // tab content panel
    MakeWidget({WW - 25,      19}, {24, 24}, WindowWidgetType::FlatBtn,   WindowColour::Secondary, SPR_RENAME,         STR_CHANGE_BANNER_TEXT_TIP     ), // change banner button
    MakeWidget({WW - 25,      43}, {24, 24}, WindowWidgetType::FlatBtn,   WindowColour::Secondary, SPR_NO_ENTRY,       STR_SET_AS_NO_ENTRY_BANNER_TIP ), // no entry button
    MakeWidget({WW - 25,      67}, {24, 24}, WindowWidgetType::FlatBtn,   WindowColour::Secondary, SPR_DEMOLISH,       STR_DEMOLISH_BANNER_TIP        ), // demolish button
    MakeWidget({      5, WH - 16}, {12, 12}, WindowWidgetType::ColourBtn, WindowColour::Secondary, 0xFFFFFFFF,         STR_SELECT_MAIN_SIGN_COLOUR_TIP), // high money
    MakeWidget({     43, WH - 16}, {39, 12}, WindowWidgetType::DropdownMenu,  WindowColour::Secondary                                                     ), // high money
    MakeWidget({     70, WH - 15}, {11, 10}, WindowWidgetType::Button,    WindowColour::Secondary, STR_DROPDOWN_GLYPH, STR_SELECT_TEXT_COLOUR_TIP     ), // high money
    WIDGETS_END,
};

// clang-format on

class BannerWindow final : public Window
{
private:
    CoordsXYZ _bannerViewPos;

    void CreateViewport()
    {
        rct_widget* viewportWidget = &window_banner_widgets[WIDX_VIEWPORT];
        viewport_create(
            this, windowPos + ScreenCoordsXY{ viewportWidget->left + 1, viewportWidget->top + 1 },
            (viewportWidget->width()) - 1, (viewportWidget->height()) - 1, Focus(_bannerViewPos));

        if (viewport != nullptr)
            viewport->flags = gConfigGeneral.always_show_gridlines ? VIEWPORT_FLAG_GRIDLINES : 0;
        Invalidate();
    }

    BannerIndex GetBannerIndex() const
    {
        return BannerIndex::FromUnderlying(number);
    }

    BannerElement* GetBannerElement()
    {
        auto* banner = GetBanner(GetBannerIndex());
        if (banner == nullptr)
        {
            return nullptr;
        }

        TileElement* tileElement = map_get_first_element_at(banner->position);
        if (tileElement == nullptr)
        {
            return nullptr;
        }

        do
        {
            auto* bannerElement = tileElement->AsBanner();
            if (bannerElement == nullptr)
            {
                continue;
            }
            if (bannerElement->GetIndex() == GetBannerIndex())
            {
                return bannerElement;
            }
        } while (!(tileElement++)->IsLastForTile());

        return nullptr;
    }

public:
    void OnOpen() override
    {
        widgets = window_banner_widgets;
        WindowInitScrollWidgets(this);
    }

    void Initialise(rct_windownumber _number)
    {
        number = _number;
        auto* banner = GetBanner(BannerIndex::FromUnderlying(number));

        auto* bannerElement = GetBannerElement();
        if (bannerElement == nullptr)
            return;

        _bannerViewPos = CoordsXYZ{ banner->position.ToCoordsXY().ToTileCentre(), bannerElement->GetBaseZ() };
        CreateViewport();
    }

    void OnMouseDown(rct_widgetindex widgetIndex) override
    {
        rct_widget* widget = &widgets[widgetIndex];
        auto* banner = GetBanner(GetBannerIndex());
        if (banner == nullptr)
        {
            Close();
            return;
        }
        switch (widgetIndex)
        {
            case WIDX_MAIN_COLOUR:
                WindowDropdownShowColour(this, widget, TRANSLUCENT(colours[1]), banner->colour);
                break;
            case WIDX_TEXT_COLOUR_DROPDOWN_BUTTON:

                for (int32_t i = 0; i < 13; ++i)
                {
                    gDropdownItems[i].Format = STR_DROPDOWN_MENU_LABEL;
                    gDropdownItems[i].Args = BannerColouredTextFormats[i + 1];
                }

                // Switch to the dropdown box widget.
                widget--;

                WindowDropdownShowTextCustomWidth(
                    { widget->left + windowPos.x, widget->top + windowPos.y }, widget->height() + 1, colours[1], 0,
                    Dropdown::Flag::StayOpen, 13, widget->width() - 3);

                Dropdown::SetChecked(banner->text_colour - 1, true);
                break;
        }
    }

    void OnMouseUp(rct_widgetindex widgetIndex) override
    {
        auto* banner = GetBanner(GetBannerIndex());
        if (banner == nullptr)
        {
            Close();
            return;
        }
        switch (widgetIndex)
        {
            case WIDX_CLOSE:
                Close();
                break;
            case WIDX_BANNER_DEMOLISH:
            {
                auto* bannerElement = GetBannerElement();
                if (bannerElement == nullptr)
                    break;

                auto bannerRemoveAction = BannerRemoveAction(
                    { banner->position.ToCoordsXY(), bannerElement->GetBaseZ(), bannerElement->GetPosition() });
                GameActions::Execute(&bannerRemoveAction);
                break;
            }
            case WIDX_BANNER_TEXT:
                WindowTextInputRawOpen(
                    this, WIDX_BANNER_TEXT, STR_BANNER_TEXT, STR_ENTER_BANNER_TEXT, {}, banner->GetText().c_str(), 32);
                break;
            case WIDX_BANNER_NO_ENTRY:
            {
                textinput_cancel();
                auto bannerSetStyle = BannerSetStyleAction(
                    BannerSetStyleType::NoEntry, GetBannerIndex(), banner->flags ^ BANNER_FLAG_NO_ENTRY);
                GameActions::Execute(&bannerSetStyle);
                break;
            }
        }
    }

    void OnDropdown(rct_widgetindex widgetIndex, int32_t dropdownIndex) override
    {
        switch (widgetIndex)
        {
            case WIDX_MAIN_COLOUR:
            {
                if (dropdownIndex == -1)
                    break;

                auto bannerSetStyle = BannerSetStyleAction(BannerSetStyleType::PrimaryColour, GetBannerIndex(), dropdownIndex);
                GameActions::Execute(&bannerSetStyle);
                break;
            }
            case WIDX_TEXT_COLOUR_DROPDOWN_BUTTON:
            {
                if (dropdownIndex == -1)
                    break;
                auto bannerSetStyle = BannerSetStyleAction(BannerSetStyleType::TextColour, GetBannerIndex(), dropdownIndex + 1);
                GameActions::Execute(&bannerSetStyle);
                break;
            }
        }
    }

    void OnTextInput(rct_widgetindex widgetIndex, std::string_view text) override
    {
        if (widgetIndex == WIDX_BANNER_TEXT)
        {
            auto bannerSetNameAction = BannerSetNameAction(GetBannerIndex(), std::string(text));
            GameActions::Execute(&bannerSetNameAction);
        }
    }

    void OnViewportRotate() override
    {
        RemoveViewport();
        CreateViewport();
    }

    void OnDraw(rct_drawpixelinfo& dpi) override
    {
        DrawWidgets(dpi);

        if (viewport != nullptr)
        {
            window_draw_viewport(&dpi, this);
        }
    }

    void OnPrepareDraw() override
    {
        auto* banner = GetBanner(GetBannerIndex());
        if (banner == nullptr)
        {
            return;
        }
        rct_widget* colourBtn = &window_banner_widgets[WIDX_MAIN_COLOUR];
        colourBtn->type = WindowWidgetType::Empty;

        auto* bannerEntry = get_banner_entry(banner->type);
        if (bannerEntry != nullptr && (bannerEntry->flags & BANNER_ENTRY_FLAG_HAS_PRIMARY_COLOUR))
        {
            colourBtn->type = WindowWidgetType::ColourBtn;
        }
        pressed_widgets &= ~(1ULL << WIDX_BANNER_NO_ENTRY);
        disabled_widgets &= ~(
            (1ULL << WIDX_BANNER_TEXT) | (1ULL << WIDX_TEXT_COLOUR_DROPDOWN) | (1ULL << WIDX_TEXT_COLOUR_DROPDOWN_BUTTON));
        if (banner->flags & BANNER_FLAG_NO_ENTRY)
        {
            pressed_widgets |= (1ULL << WIDX_BANNER_NO_ENTRY);
            disabled_widgets |= (1ULL << WIDX_BANNER_TEXT) | (1ULL << WIDX_TEXT_COLOUR_DROPDOWN)
                | (1ULL << WIDX_TEXT_COLOUR_DROPDOWN_BUTTON);
        }
        colourBtn->image = SPRITE_ID_PALETTE_COLOUR_1(banner->colour) | IMAGE_TYPE_TRANSPARENT | SPR_PALETTE_BTN;
        rct_widget* dropDownWidget = &window_banner_widgets[WIDX_TEXT_COLOUR_DROPDOWN];
        dropDownWidget->text = BannerColouredTextFormats[banner->text_colour];
    }
};

/**
 *
 *  rct2: 0x006BA305
 */
rct_window* WindowBannerOpen(rct_windownumber number)
{
    auto w = static_cast<BannerWindow*>(window_bring_to_front_by_number(WC_BANNER, number));

    if (w != nullptr)
        return w;

    w = WindowCreate<BannerWindow>(WC_BANNER, WW, WH, 0);

    if (w != nullptr)
        w->Initialise(number);

    return w;
}

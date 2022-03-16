/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include <algorithm>
#include <openrct2-ui/interface/Widget.h>
#include <openrct2-ui/windows/Window.h>
#include <openrct2/Context.h>
#include <openrct2/core/Guard.hpp>
#include <openrct2/localisation/Language.h>
#include <openrct2/localisation/Localisation.h>
#include <openrct2/localisation/LocalisationService.h>
#include <openrct2/paint/Paint.h>
#include <openrct2/paint/tile_element/Paint.TileElement.h>
#include <openrct2/ride/TrackPaint.h>

static int32_t ResizeLanguage = LANGUAGE_UNDEFINED;

// clang-format off
enum WindowDebugPaintWidgetIdx
{
    WIDX_BACKGROUND,
    WIDX_TOGGLE_SHOW_WIDE_PATHS,
    WIDX_TOGGLE_SHOW_BLOCKED_TILES,
    WIDX_TOGGLE_SHOW_SEGMENT_HEIGHTS,
    WIDX_TOGGLE_SHOW_BOUND_BOXES,
    WIDX_TOGGLE_SHOW_DIRTY_VISUALS,
};

constexpr int32_t WINDOW_WIDTH = 200;
constexpr int32_t WINDOW_HEIGHT = 8 + 15 + 15 + 15 + 15 + 11 + 8;

static rct_widget window_debug_paint_widgets[] = {
    MakeWidget({0,          0}, {WINDOW_WIDTH, WINDOW_HEIGHT}, WindowWidgetType::Frame,    WindowColour::Primary                                        ),
    MakeWidget({8, 8 + 15 * 0}, {         185,            12}, WindowWidgetType::Checkbox, WindowColour::Secondary, STR_DEBUG_PAINT_SHOW_WIDE_PATHS     ),
    MakeWidget({8, 8 + 15 * 1}, {         185,            12}, WindowWidgetType::Checkbox, WindowColour::Secondary, STR_DEBUG_PAINT_SHOW_BLOCKED_TILES  ),
    MakeWidget({8, 8 + 15 * 2}, {         185,            12}, WindowWidgetType::Checkbox, WindowColour::Secondary, STR_DEBUG_PAINT_SHOW_SEGMENT_HEIGHTS),
    MakeWidget({8, 8 + 15 * 3}, {         185,            12}, WindowWidgetType::Checkbox, WindowColour::Secondary, STR_DEBUG_PAINT_SHOW_BOUND_BOXES    ),
    MakeWidget({8, 8 + 15 * 4}, {         185,            12}, WindowWidgetType::Checkbox, WindowColour::Secondary, STR_DEBUG_PAINT_SHOW_DIRTY_VISUALS  ),
    WIDGETS_END,
};

static void WindowDebugPaintMouseup(rct_window * w, rct_widgetindex widgetIndex);
static void WindowDebugPaintInvalidate(rct_window * w);
static void WindowDebugPaintPaint(rct_window * w, rct_drawpixelinfo * dpi);

static rct_window_event_list window_debug_paint_events([](auto& events)
{
    events.mouse_up = &WindowDebugPaintMouseup;
    events.invalidate = &WindowDebugPaintInvalidate;
    events.paint = &WindowDebugPaintPaint;
});
// clang-format on

rct_window* WindowDebugPaintOpen()
{
    rct_window* window;

    // Check if window is already open
    window = window_find_by_class(WC_DEBUG_PAINT);
    if (window != nullptr)
        return window;

    window = WindowCreate(
        ScreenCoordsXY(16, context_get_height() - 16 - 33 - WINDOW_HEIGHT), WINDOW_WIDTH, WINDOW_HEIGHT,
        &window_debug_paint_events, WC_DEBUG_PAINT, WF_STICK_TO_FRONT | WF_TRANSPARENT);

    window->widgets = window_debug_paint_widgets;
    WindowInitScrollWidgets(window);
    window_push_others_below(window);

    window->colours[0] = TRANSLUCENT(COLOUR_BLACK);
    window->colours[1] = COLOUR_GREY;

    ResizeLanguage = LANGUAGE_UNDEFINED;
    return window;
}

static void WindowDebugPaintMouseup([[maybe_unused]] rct_window* w, rct_widgetindex widgetIndex)
{
    switch (widgetIndex)
    {
        case WIDX_TOGGLE_SHOW_WIDE_PATHS:
            gPaintWidePathsAsGhost = !gPaintWidePathsAsGhost;
            gfx_invalidate_screen();
            break;

        case WIDX_TOGGLE_SHOW_BLOCKED_TILES:
            gPaintBlockedTiles = !gPaintBlockedTiles;
            gfx_invalidate_screen();
            break;

        case WIDX_TOGGLE_SHOW_SEGMENT_HEIGHTS:
            gShowSupportSegmentHeights = !gShowSupportSegmentHeights;
            gfx_invalidate_screen();
            break;

        case WIDX_TOGGLE_SHOW_BOUND_BOXES:
            gPaintBoundingBoxes = !gPaintBoundingBoxes;
            gfx_invalidate_screen();
            break;

        case WIDX_TOGGLE_SHOW_DIRTY_VISUALS:
            gShowDirtyVisuals = !gShowDirtyVisuals;
            gfx_invalidate_screen();
            break;
    }
}

static void WindowDebugPaintInvalidate(rct_window* w)
{
    const auto& ls = OpenRCT2::GetContext()->GetLocalisationService();
    const auto currentLanguage = ls.GetCurrentLanguage();
    if (ResizeLanguage != currentLanguage)
    {
        ResizeLanguage = currentLanguage;
        w->Invalidate();

        // Find the width of the longest string
        int16_t newWidth = 0;
        for (size_t widgetIndex = WIDX_TOGGLE_SHOW_WIDE_PATHS; widgetIndex <= WIDX_TOGGLE_SHOW_DIRTY_VISUALS; widgetIndex++)
        {
            auto stringIdx = w->widgets[widgetIndex].text;
            auto string = ls.GetString(stringIdx);
            Guard::ArgumentNotNull(string);
            auto width = gfx_get_string_width(string, FontSpriteBase::MEDIUM);
            newWidth = std::max<int16_t>(width, newWidth);
        }

        // Add padding for both sides (8) and the offset for the text after the checkbox (15)
        newWidth += 8 * 2 + 15;

        w->width = newWidth;
        w->max_width = newWidth;
        w->min_width = newWidth;
        w->widgets[WIDX_BACKGROUND].right = newWidth - 1;
        w->widgets[WIDX_TOGGLE_SHOW_WIDE_PATHS].right = newWidth - 8;
        w->widgets[WIDX_TOGGLE_SHOW_BLOCKED_TILES].right = newWidth - 8;
        w->widgets[WIDX_TOGGLE_SHOW_SEGMENT_HEIGHTS].right = newWidth - 8;
        w->widgets[WIDX_TOGGLE_SHOW_BOUND_BOXES].right = newWidth - 8;
        w->widgets[WIDX_TOGGLE_SHOW_DIRTY_VISUALS].right = newWidth - 8;

        w->Invalidate();
    }

    WidgetSetCheckboxValue(w, WIDX_TOGGLE_SHOW_WIDE_PATHS, gPaintWidePathsAsGhost);
    WidgetSetCheckboxValue(w, WIDX_TOGGLE_SHOW_BLOCKED_TILES, gPaintBlockedTiles);
    WidgetSetCheckboxValue(w, WIDX_TOGGLE_SHOW_SEGMENT_HEIGHTS, gShowSupportSegmentHeights);
    WidgetSetCheckboxValue(w, WIDX_TOGGLE_SHOW_BOUND_BOXES, gPaintBoundingBoxes);
    WidgetSetCheckboxValue(w, WIDX_TOGGLE_SHOW_DIRTY_VISUALS, gShowDirtyVisuals);
}

static void WindowDebugPaintPaint(rct_window* w, rct_drawpixelinfo* dpi)
{
    WindowDrawWidgets(w, dpi);
}

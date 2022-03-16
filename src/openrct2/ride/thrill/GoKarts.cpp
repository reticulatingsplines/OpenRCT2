/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "../../common.h"
#include "../../interface/Viewport.h"
#include "../../paint/Paint.h"
#include "../../paint/Supports.h"
#include "../../world/Map.h"
#include "../Ride.h"
#include "../Track.h"
#include "../TrackPaint.h"
#include "../VehiclePaint.h"

enum
{
    SPR_GO_KARTS_FLAT_SW_NE = 20752,
    SPR_GO_KARTS_FLAT_NW_SE = 20753,
    SPR_GO_KARTS_FLAT_FRONT_SW_NE = 20754,
    SPR_GO_KARTS_FLAT_FRONT_NW_SE = 20755,
    SPR_GO_KARTS_STARTING_GRID_END_SW_NE = 20756,
    SPR_GO_KARTS_STARTING_GRID_END_NW_SE = 20757,
    SPR_GO_KARTS_STARTING_GRID_END_NE_SW = 20758,
    SPR_GO_KARTS_STARTING_GRID_END_SE_NW = 20759,
    SPR_GO_KARTS_STARTING_GRID_END_FRONT_SW_NE = 20760,
    SPR_GO_KARTS_STARTING_GRID_END_FRONT_NW_SE = 20761,
    SPR_GO_KARTS_STARTING_GRID_END_FRONT_NE_SW = 20762,
    SPR_GO_KARTS_STARTING_GRID_END_FRONT_SE_NW = 20763,
    SPR_GO_KARTS_STARTING_GRID_SW_NE = 20764,
    SPR_GO_KARTS_STARTING_GRID_NW_SE = 20765,
    SPR_GO_KARTS_STARTING_GRID_NE_SW = 20766,
    SPR_GO_KARTS_STARTING_GRID_SE_NW = 20767,
    SPR_GO_KARTS_STARTING_GRID_FRONT_SW_NE = 20768,
    SPR_GO_KARTS_STARTING_GRID_FRONT_NW_SE = 20769,
    SPR_GO_KARTS_STARTING_GRID_FRONT_NE_SW = 20770,
    SPR_GO_KARTS_STARTING_GRID_FRONT_SE_NW = 20771,
    SPR_GO_KARTS_FLAT_TO_25_DEG_UP_SW_NE = 20772,
    SPR_GO_KARTS_FLAT_TO_25_DEG_UP_NW_SE = 20773,
    SPR_GO_KARTS_FLAT_TO_25_DEG_UP_NE_SW = 20774,
    SPR_GO_KARTS_FLAT_TO_25_DEG_UP_SE_NW = 20775,
    SPR_GO_KARTS_25_DEG_UP_TO_FLAT_SW_NE = 20776,
    SPR_GO_KARTS_25_DEG_UP_TO_FLAT_NW_SE = 20777,
    SPR_GO_KARTS_25_DEG_UP_TO_FLAT_NE_SW = 20778,
    SPR_GO_KARTS_25_DEG_UP_TO_FLAT_SE_NW = 20779,
    SPR_GO_KARTS_25_DEG_UP_SW_NE = 20780,
    SPR_GO_KARTS_25_DEG_UP_NW_SE = 20781,
    SPR_GO_KARTS_25_DEG_UP_NE_SW = 20782,
    SPR_GO_KARTS_25_DEG_UP_SE_NW = 20783,
    SPR_GO_KARTS_FLAT_TO_25_DEG_UP_FRONT_SW_NE = 20784,
    SPR_GO_KARTS_FLAT_TO_25_DEG_UP_FRONT_NW_SE = 20785,
    SPR_GO_KARTS_FLAT_TO_25_DEG_UP_FRONT_NE_SW = 20786,
    SPR_GO_KARTS_FLAT_TO_25_DEG_UP_FRONT_SE_NW = 20787,
    SPR_GO_KARTS_25_DEG_UP_TO_FLAT_FRONT_SW_NE = 20788,
    SPR_GO_KARTS_25_DEG_UP_TO_FLAT_FRONT_NW_SE = 20789,
    SPR_GO_KARTS_25_DEG_UP_TO_FLAT_FRONT_NE_SW = 20790,
    SPR_GO_KARTS_25_DEG_UP_TO_FLAT_FRONT_SE_NW = 20791,
    SPR_GO_KARTS_25_DEG_UP_FRONT_SW_NE = 20792,
    SPR_GO_KARTS_25_DEG_UP_FRONT_NW_SE = 20793,
    SPR_GO_KARTS_25_DEG_UP_FRONT_NE_SW = 20794,
    SPR_GO_KARTS_25_DEG_UP_FRONT_SE_NW = 20795,
    SPR_GO_KARTS_FLAT_QUARTER_TURN_1_TILE_SW_NW = 20796,
    SPR_GO_KARTS_FLAT_QUARTER_TURN_1_TILE_NW_NE = 20797,
    SPR_GO_KARTS_FLAT_QUARTER_TURN_1_TILE_NE_SE = 20798,
    SPR_GO_KARTS_FLAT_QUARTER_TURN_1_TILE_SE_SW = 20799,
    SPR_GO_KARTS_FLAT_QUARTER_TURN_1_TILE_EDGE_A_SW_NW = 20800,
    SPR_GO_KARTS_FLAT_QUARTER_TURN_1_TILE_EDGE_A_NW_NE = 20801,
    SPR_GO_KARTS_FLAT_QUARTER_TURN_1_TILE_EDGE_A_NE_SE = 20802,
    SPR_GO_KARTS_FLAT_QUARTER_TURN_1_TILE_EDGE_A_SE_SW = 20803,
    SPR_GO_KARTS_FLAT_QUARTER_TURN_1_TILE_EDGE_B_SW_NW = 20804, // Empty
    SPR_GO_KARTS_FLAT_QUARTER_TURN_1_TILE_EDGE_B_NW_NE = 20805,
    SPR_GO_KARTS_FLAT_QUARTER_TURN_1_TILE_EDGE_B_NE_SE = 20806,
    SPR_GO_KARTS_FLAT_QUARTER_TURN_1_TILE_EDGE_B_SE_SW = 20807,
    SPR_GO_KARTS_START_POLE_RED_SW_NE = 20808,
    SPR_GO_KARTS_START_POLE_GREEN_SW_NE = 20809,
    SPR_GO_KARTS_START_POLE_NW_SE = 20810,
    SPR_GO_KARTS_START_POLE_NE_SW = 20811,
    SPR_GO_KARTS_START_POLE_RED_SE_NW = 20812,
    SPR_GO_KARTS_START_POLE_GREEN_SE_NW = 20813,
    SPR_GO_KARTS_START_LIGHTS_RED_SW_NE = 20814,
    SPR_GO_KARTS_START_LIGHTS_GREEN_SW_NE = 20815,
    SPR_GO_KARTS_START_LIGHTS_NW_SE = 20816,
    SPR_GO_KARTS_START_LIGHTS_NE_SW = 20817,
    SPR_GO_KARTS_START_LIGHTS_RED_SE_NW = 20818,
    SPR_GO_KARTS_START_LIGHTS_GREEN_SE_NW = 20819,
};

static constexpr const uint32_t go_karts_track_pieces_starting_grid_end[4][2] = {
    { SPR_GO_KARTS_STARTING_GRID_END_SW_NE, SPR_GO_KARTS_STARTING_GRID_END_FRONT_SW_NE },
    { SPR_GO_KARTS_STARTING_GRID_END_NW_SE, SPR_GO_KARTS_STARTING_GRID_END_FRONT_NW_SE },
    { SPR_GO_KARTS_STARTING_GRID_END_NE_SW, SPR_GO_KARTS_STARTING_GRID_END_FRONT_NE_SW },
    { SPR_GO_KARTS_STARTING_GRID_END_SE_NW, SPR_GO_KARTS_STARTING_GRID_END_FRONT_SE_NW },
};

static constexpr const uint32_t go_karts_track_pieces_starting_grid[4][2] = {
    { SPR_GO_KARTS_STARTING_GRID_SW_NE, SPR_GO_KARTS_STARTING_GRID_FRONT_SW_NE },
    { SPR_GO_KARTS_STARTING_GRID_NW_SE, SPR_GO_KARTS_STARTING_GRID_FRONT_NW_SE },
    { SPR_GO_KARTS_STARTING_GRID_NE_SW, SPR_GO_KARTS_STARTING_GRID_FRONT_NE_SW },
    { SPR_GO_KARTS_STARTING_GRID_SE_NW, SPR_GO_KARTS_STARTING_GRID_FRONT_SE_NW },
};

static constexpr const uint32_t go_karts_track_pieces_25_deg_up[4][2] = {
    { SPR_GO_KARTS_25_DEG_UP_SW_NE, SPR_GO_KARTS_25_DEG_UP_FRONT_SW_NE },
    { SPR_GO_KARTS_25_DEG_UP_NW_SE, SPR_GO_KARTS_25_DEG_UP_FRONT_NW_SE },
    { SPR_GO_KARTS_25_DEG_UP_NE_SW, SPR_GO_KARTS_25_DEG_UP_FRONT_NE_SW },
    { SPR_GO_KARTS_25_DEG_UP_SE_NW, SPR_GO_KARTS_25_DEG_UP_FRONT_SE_NW },
};

static constexpr const uint32_t go_karts_track_pieces_flat_to_25_deg_up[4][2] = {
    { SPR_GO_KARTS_FLAT_TO_25_DEG_UP_SW_NE, SPR_GO_KARTS_FLAT_TO_25_DEG_UP_FRONT_SW_NE },
    { SPR_GO_KARTS_FLAT_TO_25_DEG_UP_NW_SE, SPR_GO_KARTS_FLAT_TO_25_DEG_UP_FRONT_NW_SE },
    { SPR_GO_KARTS_FLAT_TO_25_DEG_UP_NE_SW, SPR_GO_KARTS_FLAT_TO_25_DEG_UP_FRONT_NE_SW },
    { SPR_GO_KARTS_FLAT_TO_25_DEG_UP_SE_NW, SPR_GO_KARTS_FLAT_TO_25_DEG_UP_FRONT_SE_NW },
};

static constexpr const uint32_t go_karts_track_pieces_25_deg_up_to_flat[4][2] = {
    { SPR_GO_KARTS_25_DEG_UP_TO_FLAT_SW_NE, SPR_GO_KARTS_25_DEG_UP_TO_FLAT_FRONT_SW_NE },
    { SPR_GO_KARTS_25_DEG_UP_TO_FLAT_NW_SE, SPR_GO_KARTS_25_DEG_UP_TO_FLAT_FRONT_NW_SE },
    { SPR_GO_KARTS_25_DEG_UP_TO_FLAT_NE_SW, SPR_GO_KARTS_25_DEG_UP_TO_FLAT_FRONT_NE_SW },
    { SPR_GO_KARTS_25_DEG_UP_TO_FLAT_SE_NW, SPR_GO_KARTS_25_DEG_UP_TO_FLAT_FRONT_SE_NW },
};

/** rct2: 0x0074A748 */
static void paint_go_karts_track_flat(
    paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TrackElement& trackElement)
{
    uint32_t imageId;
    if (direction == 0 || direction == 2)
    {
        imageId = SPR_GO_KARTS_FLAT_SW_NE | session.TrackColours[SCHEME_TRACK];
        PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 32, 28, 1 }, { 0, 2, height });

        imageId = SPR_GO_KARTS_FLAT_FRONT_SW_NE | session.TrackColours[SCHEME_TRACK];
        PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 32, 1, 3 }, { 0, 29, height + 2 });

        paint_util_push_tunnel_left(session, height, TUNNEL_SQUARE_FLAT);
    }
    else
    {
        imageId = SPR_GO_KARTS_FLAT_NW_SE | session.TrackColours[SCHEME_TRACK];
        PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 28, 32, 1 }, { 2, 0, height });

        imageId = SPR_GO_KARTS_FLAT_FRONT_NW_SE | session.TrackColours[SCHEME_TRACK];
        PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 1, 32, 3 }, { 29, 0, height + 2 });

        paint_util_push_tunnel_right(session, height, TUNNEL_SQUARE_FLAT);
    }

    wooden_a_supports_paint_setup(session, (direction & 1), 0, height, session.TrackColours[SCHEME_SUPPORTS]);

    paint_util_set_segment_support_height(session, SEGMENTS_ALL, 0xFFFF, 0);
    paint_util_set_general_support_height(session, height + 32, 0x20);
}

/** rct2: 0x0074A758 */
static void paint_go_karts_track_25_deg_up(
    paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TrackElement& trackElement)
{
    uint32_t imageId;
    paint_struct* ps;

    imageId = go_karts_track_pieces_25_deg_up[direction][0] | session.TrackColours[SCHEME_TRACK];
    if (direction == 0 || direction == 2)
    {
        ps = PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 32, 28, 1 }, { 0, 2, height });
    }
    else
    {
        ps = PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 28, 32, 1 }, { 2, 0, height });
    }

    imageId = go_karts_track_pieces_25_deg_up[direction][1] | session.TrackColours[SCHEME_TRACK];
    if (direction == 0 || direction == 2)
    {
        PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 32, 1, 11 }, { 0, 29, height + 2 });
    }
    else
    {
        PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 1, 32, 11 }, { 29, 0, height + 2 });
    }

    session.WoodenSupportsPrependTo = ps;

    switch (direction)
    {
        case 0:
            wooden_a_supports_paint_setup(session, 0, 9, height, session.TrackColours[SCHEME_SUPPORTS]);
            paint_util_push_tunnel_left(session, height - 8, TUNNEL_SQUARE_7);
            break;
        case 1:
            wooden_a_supports_paint_setup(session, 1, 10, height, session.TrackColours[SCHEME_SUPPORTS]);
            paint_util_push_tunnel_right(session, height + 8, TUNNEL_SQUARE_8);
            break;
        case 2:
            wooden_a_supports_paint_setup(session, 0, 11, height, session.TrackColours[SCHEME_SUPPORTS]);
            paint_util_push_tunnel_left(session, height + 8, TUNNEL_SQUARE_8);
            break;
        case 3:
            wooden_a_supports_paint_setup(session, 1, 12, height, session.TrackColours[SCHEME_SUPPORTS]);
            paint_util_push_tunnel_right(session, height - 8, TUNNEL_SQUARE_7);
            break;
    }

    paint_util_set_segment_support_height(session, SEGMENTS_ALL, 0xFFFF, 0);
    paint_util_set_general_support_height(session, height + 56, 0x20);
}

/** rct2: 0x0074A768 */
static void paint_go_karts_track_flat_to_25_deg_up(
    paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TrackElement& trackElement)
{
    uint32_t imageId;
    paint_struct* ps;

    imageId = go_karts_track_pieces_flat_to_25_deg_up[direction][0] | session.TrackColours[SCHEME_TRACK];
    if (direction == 0 || direction == 2)
    {
        ps = PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 32, 28, 1 }, { 0, 2, height });
    }
    else
    {
        ps = PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 28, 32, 1 }, { 2, 0, height });
    }

    imageId = go_karts_track_pieces_flat_to_25_deg_up[direction][1] | session.TrackColours[SCHEME_TRACK];
    if (direction == 0 || direction == 2)
    {
        PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 32, 1, 11 }, { 0, 29, height + 2 });
    }
    else
    {
        PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 1, 32, 11 }, { 29, 0, height + 2 });
    }

    session.WoodenSupportsPrependTo = ps;

    switch (direction)
    {
        case 0:
            wooden_a_supports_paint_setup(session, 0, 1, height, session.TrackColours[SCHEME_SUPPORTS]);
            paint_util_push_tunnel_left(session, height, TUNNEL_SQUARE_FLAT);
            break;
        case 1:
            wooden_a_supports_paint_setup(session, 1, 2, height, session.TrackColours[SCHEME_SUPPORTS]);
            paint_util_push_tunnel_right(session, height, TUNNEL_SQUARE_8);
            break;
        case 2:
            wooden_a_supports_paint_setup(session, 0, 3, height, session.TrackColours[SCHEME_SUPPORTS]);
            paint_util_push_tunnel_left(session, height, TUNNEL_SQUARE_8);
            break;
        case 3:
            wooden_a_supports_paint_setup(session, 1, 4, height, session.TrackColours[SCHEME_SUPPORTS]);
            paint_util_push_tunnel_right(session, height, TUNNEL_SQUARE_FLAT);
            break;
    }

    paint_util_set_segment_support_height(session, SEGMENTS_ALL, 0xFFFF, 0);
    paint_util_set_general_support_height(session, height + 48, 0x20);
}

/** rct2: 0x */
static void paint_go_karts_track_25_deg_up_to_flat(
    paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TrackElement& trackElement)
{
    uint32_t imageId;
    paint_struct* ps;

    imageId = go_karts_track_pieces_25_deg_up_to_flat[direction][0] | session.TrackColours[SCHEME_TRACK];
    if (direction == 0 || direction == 2)
    {
        ps = PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 32, 28, 1 }, { 0, 2, height });
    }
    else
    {
        ps = PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 28, 32, 1 }, { 2, 0, height });
    }

    imageId = go_karts_track_pieces_25_deg_up_to_flat[direction][1] | session.TrackColours[SCHEME_TRACK];
    if (direction == 0 || direction == 2)
    {
        PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 32, 1, 11 }, { 0, 29, height + 2 });
    }
    else
    {
        PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 1, 32, 11 }, { 29, 0, height + 2 });
    }

    session.WoodenSupportsPrependTo = ps;

    switch (direction)
    {
        case 0:
            wooden_a_supports_paint_setup(session, 0, 5, height, session.TrackColours[SCHEME_SUPPORTS]);
            paint_util_push_tunnel_left(session, height - 8, TUNNEL_SQUARE_FLAT);
            break;
        case 1:
            wooden_a_supports_paint_setup(session, 1, 6, height, session.TrackColours[SCHEME_SUPPORTS]);
            paint_util_push_tunnel_right(session, height + 8, TUNNEL_14);
            break;
        case 2:
            wooden_a_supports_paint_setup(session, 0, 7, height, session.TrackColours[SCHEME_SUPPORTS]);
            paint_util_push_tunnel_left(session, height + 8, TUNNEL_14);
            break;
        case 3:
            wooden_a_supports_paint_setup(session, 1, 8, height, session.TrackColours[SCHEME_SUPPORTS]);
            paint_util_push_tunnel_right(session, height - 8, TUNNEL_SQUARE_FLAT);
            break;
    }

    paint_util_set_segment_support_height(session, SEGMENTS_ALL, 0xFFFF, 0);
    paint_util_set_general_support_height(session, height + 40, 0x20);
}

/** rct2: 0x0074A788 */
static void paint_go_karts_track_25_deg_down(
    paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TrackElement& trackElement)
{
    paint_go_karts_track_25_deg_up(session, ride, trackSequence, (direction + 2) % 4, height, trackElement);
}

/** rct2: 0x0074A798 */
static void paint_go_karts_track_flat_to_25_deg_down(
    paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TrackElement& trackElement)
{
    paint_go_karts_track_25_deg_up_to_flat(session, ride, trackSequence, (direction + 2) % 4, height, trackElement);
}

/** rct2: 0x0074A7A8 */
static void paint_go_karts_track_25_deg_down_to_flat(
    paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TrackElement& trackElement)
{
    paint_go_karts_track_flat_to_25_deg_up(session, ride, trackSequence, (direction + 2) % 4, height, trackElement);
}

/** rct2: 0x */
static void paint_go_karts_station(
    paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TrackElement& trackElement)
{
    const auto* stationObj = ride.GetStationObject();

    bool hasFence;
    uint32_t imageId;
    const uint32_t(*sprites)[2] = go_karts_track_pieces_starting_grid;

    if (trackElement.GetTrackType() == TrackElemType::EndStation)
    {
        sprites = go_karts_track_pieces_starting_grid_end;
    }

    imageId = sprites[direction][0] | session.TrackColours[SCHEME_TRACK];
    if (direction == 0 || direction == 2)
    {
        PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 32, 28, 1 }, { 0, 2, height });
    }
    else
    {
        PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 28, 32, 1 }, { 2, 0, height });
    }

    if (direction == 0 || direction == 2)
    {
        hasFence = track_paint_util_has_fence(EDGE_NW, session.MapPosition, trackElement, ride, session.CurrentRotation);
        track_paint_util_draw_station_covers(session, EDGE_NW, hasFence, stationObj, height);
    }
    else
    {
        hasFence = track_paint_util_has_fence(EDGE_NE, session.MapPosition, trackElement, ride, session.CurrentRotation);
        track_paint_util_draw_station_covers(session, EDGE_NE, hasFence, stationObj, height);
    }

    imageId = sprites[direction][1] | session.TrackColours[SCHEME_TRACK];
    if (direction == 0 || direction == 2)
    {
        PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 32, 1, 3 }, { 0, 29, height + 2 });

        paint_util_push_tunnel_left(session, height, TUNNEL_SQUARE_FLAT);
    }
    else
    {
        PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 1, 32, 3 }, { 29, 0, height + 2 });

        paint_util_push_tunnel_right(session, height, TUNNEL_SQUARE_FLAT);
    }

    if (direction == 0 || direction == 2)
    {
        hasFence = track_paint_util_has_fence(EDGE_SE, session.MapPosition, trackElement, ride, session.CurrentRotation);
        track_paint_util_draw_station_covers(session, EDGE_SE, hasFence, stationObj, height);
    }
    else
    {
        hasFence = track_paint_util_has_fence(EDGE_SW, session.MapPosition, trackElement, ride, session.CurrentRotation);
        track_paint_util_draw_station_covers(session, EDGE_SW, hasFence, stationObj, height);
    }

    if (trackElement.GetTrackType() == TrackElemType::EndStation)
    {
        const bool hasGreenLight = trackElement.HasGreenLight();

        switch (direction)
        {
            case 0:
                imageId = (hasGreenLight ? SPR_GO_KARTS_START_POLE_GREEN_SW_NE : SPR_GO_KARTS_START_POLE_RED_SW_NE)
                    | session.TrackColours[SCHEME_TRACK];
                PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 3, 3, 13 }, { 1, 1, height + 4 });

                imageId = (hasGreenLight ? SPR_GO_KARTS_START_LIGHTS_GREEN_SW_NE : SPR_GO_KARTS_START_LIGHTS_RED_SW_NE)
                    | session.TrackColours[SCHEME_TRACK];
                PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 3, 3, 13 }, { 1, 28, height + 4 });
                break;
            case 1:
                imageId = SPR_GO_KARTS_START_POLE_NW_SE | session.TrackColours[SCHEME_TRACK];
                PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 3, 3, 13 }, { 1, 28, height + 4 });

                imageId = SPR_GO_KARTS_START_LIGHTS_NW_SE | session.TrackColours[SCHEME_TRACK];
                PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 3, 3, 13 }, { 28, 28, height + 4 });
                break;
            case 2:
                imageId = SPR_GO_KARTS_START_POLE_NE_SW | session.TrackColours[SCHEME_TRACK];
                PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 3, 3, 13 }, { 28, 1, height + 4 });

                imageId = SPR_GO_KARTS_START_LIGHTS_NE_SW | session.TrackColours[SCHEME_TRACK];
                PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 3, 3, 13 }, { 28, 28, height + 4 });
                break;
            case 3:
                imageId = (hasGreenLight ? SPR_GO_KARTS_START_POLE_GREEN_SE_NW : SPR_GO_KARTS_START_POLE_RED_SE_NW)
                    | session.TrackColours[SCHEME_TRACK];
                PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 3, 3, 13 }, { 1, 1, height + 4 });

                imageId = (hasGreenLight ? SPR_GO_KARTS_START_LIGHTS_GREEN_SE_NW : SPR_GO_KARTS_START_LIGHTS_RED_SE_NW)
                    | session.TrackColours[SCHEME_TRACK];
                PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 3, 3, 13 }, { 28, 1, height + 4 });
                break;
        }
    }

    wooden_a_supports_paint_setup(session, (direction & 1), 0, height, session.TrackColours[SCHEME_SUPPORTS]);

    paint_util_set_segment_support_height(session, SEGMENTS_ALL, 0xFFFF, 0);
    paint_util_set_general_support_height(session, height + 32, 0x20);
}

/** rct2: 0x0074A7E8 */
static void paint_go_karts_track_left_quarter_turn_1_tile(
    paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TrackElement& trackElement)
{
    uint32_t imageId;

    switch (direction)
    {
        case 0:
            imageId = SPR_GO_KARTS_FLAT_QUARTER_TURN_1_TILE_NW_NE | session.TrackColours[SCHEME_TRACK];
            PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 32, 28, 1 }, { 0, 2, height });

            imageId = SPR_GO_KARTS_FLAT_QUARTER_TURN_1_TILE_EDGE_A_NW_NE | session.TrackColours[SCHEME_TRACK];
            PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 1, 1, 3 }, { 29, 2, height + 2 });

            imageId = SPR_GO_KARTS_FLAT_QUARTER_TURN_1_TILE_EDGE_B_NW_NE | session.TrackColours[SCHEME_TRACK];
            PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 16, 1, 3 }, { 14, 29, height + 2 });
            break;
        case 1:
            imageId = SPR_GO_KARTS_FLAT_QUARTER_TURN_1_TILE_NE_SE | session.TrackColours[SCHEME_TRACK];
            PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 30, 30, 1 }, { 0, 0, height });

            imageId = SPR_GO_KARTS_FLAT_QUARTER_TURN_1_TILE_EDGE_A_NE_SE | session.TrackColours[SCHEME_TRACK];
            PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 16, 1, 3 }, { 2, 29, height + 2 });

            imageId = SPR_GO_KARTS_FLAT_QUARTER_TURN_1_TILE_EDGE_B_NE_SE | session.TrackColours[SCHEME_TRACK];
            PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 1, 16, 3 }, { 29, 2, height + 2 });
            break;
        case 2:
            imageId = SPR_GO_KARTS_FLAT_QUARTER_TURN_1_TILE_SE_SW | session.TrackColours[SCHEME_TRACK];
            PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 28, 32, 1 }, { 2, 0, height });

            imageId = SPR_GO_KARTS_FLAT_QUARTER_TURN_1_TILE_EDGE_A_SE_SW | session.TrackColours[SCHEME_TRACK];
            PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 1, 1, 3 }, { 2, 2, height + 2 });

            imageId = SPR_GO_KARTS_FLAT_QUARTER_TURN_1_TILE_EDGE_B_SE_SW | session.TrackColours[SCHEME_TRACK];
            PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 1, 16, 3 }, { 29, 14, height + 2 });
            break;
        case 3:
            imageId = SPR_GO_KARTS_FLAT_QUARTER_TURN_1_TILE_SW_NW | session.TrackColours[SCHEME_TRACK];
            PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 32, 32, 1 }, { 0, 0, height });

            imageId = SPR_GO_KARTS_FLAT_QUARTER_TURN_1_TILE_EDGE_A_SW_NW | session.TrackColours[SCHEME_TRACK];
            PaintAddImageAsParent(session, imageId, { 0, 0, height }, { 1, 1, 3 }, { 29, 29, height + 2 });

            // The empty sprite isn't drawn
            break;
    }

    switch (direction)
    {
        case 0:
            paint_util_push_tunnel_left(session, height, TUNNEL_SQUARE_FLAT);
            break;
        case 2:
            paint_util_push_tunnel_right(session, height, TUNNEL_SQUARE_FLAT);
            break;
        case 3:
            paint_util_push_tunnel_left(session, height, TUNNEL_SQUARE_FLAT);
            paint_util_push_tunnel_right(session, height, TUNNEL_SQUARE_FLAT);
            break;
    }

    wooden_a_supports_paint_setup(session, (direction & 1), 0, height, session.TrackColours[SCHEME_SUPPORTS]);

    paint_util_set_segment_support_height(session, SEGMENTS_ALL, 0xFFFF, 0);
    paint_util_set_general_support_height(session, height + 32, 0x20);
}

/** rct2: 0x0074A7F8 */
static void paint_go_karts_track_right_quarter_turn_1_tile(
    paint_session& session, const Ride& ride, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TrackElement& trackElement)
{
    paint_go_karts_track_left_quarter_turn_1_tile(session, ride, trackSequence, (direction + 3) % 4, height, trackElement);
}

/**
 * rct2: 0x0074A668
 */
TRACK_PAINT_FUNCTION get_track_paint_function_go_karts(int32_t trackType)
{
    switch (trackType)
    {
        case TrackElemType::Flat:
            return paint_go_karts_track_flat;

        case TrackElemType::EndStation:
        case TrackElemType::BeginStation:
        case TrackElemType::MiddleStation:
            return paint_go_karts_station;

        case TrackElemType::Up25:
            return paint_go_karts_track_25_deg_up;
        case TrackElemType::FlatToUp25:
            return paint_go_karts_track_flat_to_25_deg_up;
        case TrackElemType::Up25ToFlat:
            return paint_go_karts_track_25_deg_up_to_flat;

        case TrackElemType::Down25:
            return paint_go_karts_track_25_deg_down;
        case TrackElemType::FlatToDown25:
            return paint_go_karts_track_flat_to_25_deg_down;
        case TrackElemType::Down25ToFlat:
            return paint_go_karts_track_25_deg_down_to_flat;

        case TrackElemType::LeftQuarterTurn1Tile:
            return paint_go_karts_track_left_quarter_turn_1_tile;
        case TrackElemType::RightQuarterTurn1Tile:
            return paint_go_karts_track_right_quarter_turn_1_tile;
    }

    return nullptr;
}

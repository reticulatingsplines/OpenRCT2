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
#include "../world/Map.h"
#include "Peep.h"

class DataSerialiser;

// The number of elements in the gStaffPatrolAreas array per staff member. Every bit in the array represents a 4x4 square.
// Right now, it's a 32-bit array like in RCT2. 32 * 128 = 4096 bits, which is also the number of 4x4 squares on a 256x256 map.
constexpr size_t STAFF_PATROL_AREA_BLOCKS_PER_LINE = MAXIMUM_MAP_SIZE_TECHNICAL / 4;
constexpr size_t STAFF_PATROL_AREA_SIZE = (STAFF_PATROL_AREA_BLOCKS_PER_LINE * STAFF_PATROL_AREA_BLOCKS_PER_LINE) / 32;

struct PatrolArea
{
    uint32_t Data[STAFF_PATROL_AREA_SIZE];
};

struct Staff : Peep
{
    static constexpr auto cEntityType = EntityType::Staff;

public:
    PatrolArea* PatrolInfo;
    StaffType AssignedStaffType;
    uint16_t MechanicTimeSinceCall; // time getting to ride to fix
    int32_t HireDate;
    uint8_t StaffOrders;
    uint8_t StaffMowingTimeout;
    union
    {
        uint16_t StaffLawnsMown;
        uint16_t StaffRidesFixed;
    };
    union
    {
        uint16_t StaffGardensWatered;
        uint16_t StaffRidesInspected;
    };
    union
    {
        uint16_t StaffLitterSwept;
        uint16_t StaffVandalsStopped;
    };
    uint16_t StaffBinsEmptied;

    void UpdateStaff(uint32_t stepsToTake);
    void Tick128UpdateStaff();
    bool IsMechanic() const;
    bool IsPatrolAreaSet(const CoordsXY& coords) const;
    bool IsLocationInPatrol(const CoordsXY& loc) const;
    bool IsLocationOnPatrolEdge(const CoordsXY& loc) const;
    bool DoPathFinding();
    uint8_t GetCostume() const;
    void SetCostume(uint8_t value);
    void SetHireDate(int32_t hireDate);
    int32_t GetHireDate() const;

    bool CanIgnoreWideFlag(const CoordsXYZ& staffPos, TileElement* path) const;

    static void ResetStats();
    void Serialise(DataSerialiser& stream);

    void ClearPatrolArea();
    void SetPatrolArea(const CoordsXY& coords, bool value);
    bool HasPatrolArea() const;

private:
    void UpdatePatrolling();
    void UpdateMowing();
    void UpdateSweeping();
    void UpdateEmptyingBin();
    void UpdateWatering();
    void UpdateAnswering();
    void UpdateFixing(int32_t steps);
    bool UpdateFixingEnterStation(Ride* ride) const;
    bool UpdateFixingMoveToBrokenDownVehicle(bool firstRun, const Ride* ride);
    bool UpdateFixingFixVehicle(bool firstRun, const Ride* ride);
    bool UpdateFixingFixVehicleMalfunction(bool firstRun, const Ride* ride);
    bool UpdateFixingMoveToStationEnd(bool firstRun, const Ride* ride);
    bool UpdateFixingFixStationEnd(bool firstRun);
    bool UpdateFixingMoveToStationStart(bool firstRun, const Ride* ride);
    bool UpdateFixingFixStationStart(bool firstRun, const Ride* ride);
    bool UpdateFixingFixStationBrakes(bool firstRun, Ride* ride);
    bool UpdateFixingMoveToStationExit(bool firstRun, const Ride* ride);
    bool UpdateFixingFinishFixOrInspect(bool firstRun, int32_t steps, Ride* ride);
    bool UpdateFixingLeaveByEntranceExit(bool firstRun, const Ride* ride);
    void UpdateRideInspected(ride_id_t rideIndex);
    void UpdateHeadingToInspect();

    bool DoHandymanPathFinding();
    bool DoMechanicPathFinding();
    bool DoEntertainerPathFinding();
    bool DoMiscPathFinding();

    Direction HandymanDirectionRandSurface(uint8_t validDirections) const;

    void EntertainerUpdateNearbyPeeps() const;

    uint8_t GetValidPatrolDirections(const CoordsXY& loc) const;
    Direction HandymanDirectionToNearestLitter() const;
    uint8_t HandymanDirectionToUncutGrass(uint8_t valid_directions) const;
    Direction DirectionSurface(Direction initialDirection) const;
    Direction DirectionPath(uint8_t validDirections, PathElement* pathElement) const;
    Direction MechanicDirectionSurface() const;
    Direction MechanicDirectionPathRand(uint8_t pathDirections) const;
    Direction MechanicDirectionPath(uint8_t validDirections, PathElement* pathElement);
    bool UpdatePatrollingFindWatering();
    bool UpdatePatrollingFindBin();
    bool UpdatePatrollingFindSweeping();
    bool UpdatePatrollingFindGrass();
};
static_assert(sizeof(Staff) <= 512);

enum STAFF_ORDERS
{
    STAFF_ORDERS_SWEEPING = (1 << 0),
    STAFF_ORDERS_WATER_FLOWERS = (1 << 1),
    STAFF_ORDERS_EMPTY_BINS = (1 << 2),
    STAFF_ORDERS_MOWING = (1 << 3),
    STAFF_ORDERS_INSPECT_RIDES = (1 << 0),
    STAFF_ORDERS_FIX_RIDES = (1 << 1)
};

enum class EntertainerCostume : uint8_t
{
    Panda,
    Tiger,
    Elephant,
    Roman,
    Gorilla,
    Snowman,
    Knight,
    Astronaut,
    Bandit,
    Sheriff,
    Pirate,

    Count
};

extern const rct_string_id StaffCostumeNames[static_cast<uint8_t>(EntertainerCostume::Count)];

extern uint16_t gStaffDrawPatrolAreas;
extern colour_t gStaffHandymanColour;
extern colour_t gStaffMechanicColour;
extern colour_t gStaffSecurityColour;

void staff_reset_modes();
void staff_update_greyed_patrol_areas();
bool staff_is_patrol_area_set_for_type(StaffType type, const CoordsXY& coords);
colour_t staff_get_colour(StaffType staffType);
bool staff_set_colour(StaffType staffType, colour_t value);
uint32_t staff_get_available_entertainer_costumes();
int32_t staff_get_available_entertainer_costume_list(EntertainerCostume* costumeList);

money32 GetStaffWage(StaffType type);
PeepSpriteType EntertainerCostumeToSprite(EntertainerCostume entertainerType);

const PatrolArea& GetMergedPatrolArea(const StaffType type);

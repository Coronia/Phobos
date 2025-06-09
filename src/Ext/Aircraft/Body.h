#pragma once

// Forward declarations
class AircraftClass;
class AbstractClass;
class BuildingClass;

// Required includes
#include <AircraftClass.h>

// TODO: Implement proper extended AircraftClass.

class AircraftExt
{
public:
	static void FireWeapon(AircraftClass* pThis, AbstractClass* pTarget);
	static bool PlaceReinforcementAircraft(AircraftClass* pThis, CellStruct edgeCell);
	static DirType GetLandingDir(AircraftClass* pThis, BuildingClass* pDock = nullptr);
};

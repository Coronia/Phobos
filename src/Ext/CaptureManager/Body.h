#pragma once

// Forward declarations
class CaptureManagerClass;
class AnimTypeClass;
class RulesClass;
class FootClass;
class HouseClass;
class AnimClass;
class TechnoClass;
class AbstractClass;
class TechnoTypeExt;

// Required includes
#include <CaptureManagerClass.h>
#include <RulesClass.h>
#include <FootClass.h>
#include <HouseClass.h>

#include <Ext/TechnoType/Body.h>

class CaptureManagerExt
{
public:
	static bool CanCapture(CaptureManagerClass* pManager, TechnoClass* pTarget);
	static bool FreeUnit(CaptureManagerClass* pManager, TechnoClass* pTarget, bool silent = false);
	static bool CaptureUnit(CaptureManagerClass* pManager, TechnoClass* pTarget, bool bRemoveFirst,
		AnimTypeClass* pControlledAnimType = RulesClass::Instance->ControlledAnimationType, bool silent = false, int threatDelay = 0);
	static bool CaptureUnit(CaptureManagerClass* pManager, AbstractClass* pTechno,
		AnimTypeClass* pControlledAnimType = RulesClass::Instance->ControlledAnimationType, int threatDelay = 0);
	static void DecideUnitFate(CaptureManagerClass* pManager, FootClass* pFoot);
};

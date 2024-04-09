#include "PhobosGlobal.h"
#include <Utilities/TemplateDef.h>

#include <JumpjetLocomotionClass.h>

#include <Ext/SWType/Body.h>
#include <Ext/House/Body.h>

//GlobalObject initial
PhobosGlobal PhobosGlobal::GlobalObject;

void PhobosGlobal::Clear()
{
	GlobalObject.Reset();
}

void PhobosGlobal::Reset()
{
	Techno_HugeBar.clear();
}

void PhobosGlobal::PointerGotInvalid(void* ptr, bool removed)
{
	GlobalObject.InvalidatePointer(ptr, removed);
}

void PhobosGlobal::InvalidatePointer(void* ptr, bool removed)
{
	if (removed)
	{
		for (auto it = Techno_HugeBar.begin(); it != Techno_HugeBar.end(); ++it)
		{
			if (it->second == ptr)
			{
				Techno_HugeBar.erase(it);

				break;
			}
		}
	}
}

//Save/Load
#pragma region save/load

template <typename T>
bool PhobosGlobal::Serialize(T& stm)
{
	return stm
		.Process(this->Techno_HugeBar)
		.Success();
}

bool PhobosGlobal::Save(PhobosStreamWriter& stm)
{
	return Serialize(stm);
}

bool PhobosGlobal::Load(PhobosStreamReader& stm)
{
	return Serialize(stm);
}

bool PhobosGlobal::SaveGlobals(PhobosStreamWriter& stm)
{
	GlobalObject.Save(stm);
	return stm.Success();
}

bool PhobosGlobal::LoadGlobals(PhobosStreamReader& stm)
{
	GlobalObject.Load(stm);
	return stm.Success();
}

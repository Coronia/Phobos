#include "InterceptorTypeClass.h"

#include <Utilities/SavegameDef.h>
#include <Utilities/TemplateDef.h>

InterceptorTypeClass::InterceptorTypeClass(TechnoTypeClass* OwnedBy) : OwnerType { OwnedBy }
	, CanTargetHouses { AffectedHouse::Enemies }
	, GuardRange {}
	, MinimumGuardRange {}
	, Weapon { 0 }
	, DeleteOnIntercept {}
	, WeaponOverride {}
	, WeaponReplaceProjectile { false }
	, WeaponCumulativeDamage { false }
	, KeepIntact { false }
	, ChangeOwner { false }
	, Retarget { InterceptorRetargetType::None }
	, Retarget_LockTarget { true }
	, Retarget_RangeOverride {}
	, Retarget_MaximumRange {}
	, Retarget_MinimumRange {}
	, Retarget_ConsiderFormerRange { true }
{ }

void InterceptorTypeClass::LoadFromINI(CCINIClass* pINI, const char* pSection)
{
	INI_EX exINI(pINI);

	this->CanTargetHouses.Read(exINI, pSection, "Interceptor.CanTargetHouses");
	this->GuardRange.Read(exINI, pSection, "Interceptor.%sGuardRange");
	this->MinimumGuardRange.Read(exINI, pSection, "Interceptor.%sMinimumGuardRange");
	this->Weapon.Read(exINI, pSection, "Interceptor.Weapon");
	this->DeleteOnIntercept.Read(exINI, pSection, "Interceptor.DeleteOnIntercept");
	this->WeaponOverride.Read<true>(exINI, pSection, "Interceptor.WeaponOverride");
	this->WeaponReplaceProjectile.Read(exINI, pSection, "Interceptor.WeaponReplaceProjectile");
	this->WeaponCumulativeDamage.Read(exINI, pSection, "Interceptor.WeaponCumulativeDamage");
	this->KeepIntact.Read(exINI, pSection, "Interceptor.KeepIntact");
	this->ChangeOwner.Read(exINI, pSection, "Interceptor.ChangeOwner");
	this->Retarget.Read(exINI, pSection, "Interceptor.Retarget");
	this->Retarget_LockTarget.Read(exINI, pSection, "Interceptor.Retarget.LockTarget");
	this->Retarget_RangeOverride.Read(exINI, pSection, "Interceptor.Retarget.RangeOverride");
	this->Retarget_MaximumRange.Read(exINI, pSection, "Interceptor.Retarget.MaximumRange");
	this->Retarget_MinimumRange.Read(exINI, pSection, "Interceptor.Retarget.MinimumRange");
	this->Retarget_ConsiderFormerRange.Read(exINI, pSection, "Interceptor.Retarget.ConsiderFormerRange");
}

#pragma region(save/load)

template <class T>
bool InterceptorTypeClass::Serialize(T& stm)
{
	return stm
		.Process(this->OwnerType)
		.Process(this->CanTargetHouses)
		.Process(this->GuardRange)
		.Process(this->MinimumGuardRange)
		.Process(this->Weapon)
		.Process(this->DeleteOnIntercept)
		.Process(this->WeaponOverride)
		.Process(this->WeaponReplaceProjectile)
		.Process(this->WeaponCumulativeDamage)
		.Process(this->KeepIntact)
		.Process(this->ChangeOwner)
		.Process(this->Retarget)
		.Process(this->Retarget_LockTarget)
		.Process(this->Retarget_RangeOverride)
		.Process(this->Retarget_MaximumRange)
		.Process(this->Retarget_MinimumRange)
		.Process(this->Retarget_ConsiderFormerRange)
		.Success();
}

bool InterceptorTypeClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool InterceptorTypeClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<InterceptorTypeClass*>(this)->Serialize(stm);
}

#pragma endregion(save/load)

#pragma once

#include <Utilities/Constructs.h>
#include <Utilities/Enum.h>
#include <Utilities/Template.h>

class InterceptorTypeClass
{
public:

	InterceptorTypeClass() = default;

	InterceptorTypeClass(TechnoTypeClass* OwnedBy);

	TechnoTypeClass* OwnerType;

	Valueable<AffectedHouse> CanTargetHouses;
	Promotable<Leptons> GuardRange;
	Promotable<Leptons> MinimumGuardRange;
	Valueable<int> Weapon;
	Nullable<WeaponTypeClass*> WeaponOverride;
	Valueable<bool> WeaponReplaceProjectile;
	Valueable<bool> WeaponCumulativeDamage;
	Valueable<bool> KeepIntact;
	Nullable<bool> DeleteOnIntercept;
	Valueable<bool> ChangeOwner;
	Valueable<InterceptorRetargetType> Retarget;
	Valueable<bool> Retarget_LockTarget;
	Nullable<Leptons> Retarget_RangeOverride;
	Nullable<Leptons> Retarget_MaximumRange;
	Nullable<Leptons> Retarget_MinimumRange;
	Valueable<bool> Retarget_ConsiderFormerRange;

	void LoadFromINI(CCINIClass* pINI, const char* pSection);
	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:

	template <typename T>
	bool Serialize(T& stm);
};

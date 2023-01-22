#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/TemplateDef.h>

class AttachEffectTypeClass final : public Enumerable<AttachEffectTypeClass>
{
public:
	Valueable<int> Duration;
	Valueable<bool> Cumulative;
	Valueable<int> Cumulative_MaxCount;
	Valueable<bool> Powered;
	Valueable<bool> DiscardOnEntry;
	Valueable<bool> PenetratesIronCurtain;
	Nullable<AnimTypeClass*> Animation;
	Valueable<bool> Animation_ResetOnReapply;
	Valueable<AttachedAnimFlag> Animation_OfflineAction;
	Valueable<AttachedAnimFlag> Animation_TemporalAction;
	Valueable<bool> Animation_UseInvokerAsOwner;
	Nullable<ColorStruct> Tint_Color;
	Valueable<double> Tint_Intensity;
	Valueable<AffectedHouse> Tint_VisibleToHouses;
	Valueable<double> FirepowerMultiplier;
	Valueable<double> ArmorMultiplier;
	Valueable<double> SpeedMultiplier;
	Valueable<double> ROFMultiplier;
	Valueable<bool> ROFMultiplier_ApplyOnCurrentTimer;
	Valueable<bool> Cloakable;
	Valueable<bool> ForceDecloak;
	Nullable<WeaponTypeClass*> RevengeWeapon;
	Valueable<AffectedHouse> RevengeWeapon_AffectsHouses;
	Valueable<double> WeaponRangeBonus;
	ValueableVector<WeaponTypeClass*> WeaponRangeBonus_AllowWeapons;
	ValueableVector<WeaponTypeClass*> WeaponRangeBonus_DisallowWeapons;

	AttachEffectTypeClass(const char* const pTitle) : Enumerable<AttachEffectTypeClass>(pTitle)
		, Duration { 0 }
		, Cumulative { false }
		, Cumulative_MaxCount { -1 }
		, Powered { false }
		, DiscardOnEntry { false }
		, PenetratesIronCurtain { false }
		, Animation {}
		, Animation_ResetOnReapply { false }
		, Animation_OfflineAction { AttachedAnimFlag::Hides }
		, Animation_TemporalAction { AttachedAnimFlag::None }
		, Animation_UseInvokerAsOwner { false }
		, Tint_Color {}
		, Tint_Intensity { 0.0 }
		, Tint_VisibleToHouses { AffectedHouse::All }
		, FirepowerMultiplier { 1.0 }
		, ArmorMultiplier { 1.0 }
		, SpeedMultiplier { 1.0 }
		, ROFMultiplier { 1.0 }
		, ROFMultiplier_ApplyOnCurrentTimer { true }
		, Cloakable { false }
		, ForceDecloak { false }
		, RevengeWeapon {}
		, RevengeWeapon_AffectsHouses { AffectedHouse::All }
		, WeaponRangeBonus { 0.0 }
		, WeaponRangeBonus_AllowWeapons {}
		, WeaponRangeBonus_DisallowWeapons {}
	{};

	bool HasTint();

	virtual ~AttachEffectTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};


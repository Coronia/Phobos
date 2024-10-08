#include "ConditionGroup.h"
#include <Ext/House/Body.h>
#include <Utilities/EnumFunctions.h>

bool ConditionGroup::CheckHouseConditions(HouseClass* pOwner, const ConditionGroup condition)
{
	// Owning house
	if (pOwner->IsControlledByHuman())
	{
		if (condition.OwnedByPlayer && condition.OnAnyCondition)
			return true;

		if (condition.OwnedByAI && !condition.OnAnyCondition)
			return false;
	}
	else
	{
		if (condition.OwnedByPlayer && !condition.OnAnyCondition)
			return false;

		if (condition.OwnedByAI && condition.OnAnyCondition)
			return true;
	}

	// Money
	if (condition.MoneyExceed >= 0)
	{
		if (pOwner->Available_Money() >= condition.MoneyExceed)
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	if (condition.MoneyBelow >= 0)
	{
		if (pOwner->Available_Money() <= condition.MoneyBelow)
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	// Power
	if (pOwner->HasLowPower())
	{
		if (condition.LowPower && condition.OnAnyCondition)
			return true;

		if (condition.FullPower && !condition.OnAnyCondition)
			return false;
	}
	else if (pOwner->HasFullPower())
	{
		if (condition.LowPower && !condition.OnAnyCondition)
			return false;

		if (condition.FullPower && condition.OnAnyCondition)
			return true;
	}

	// TechLevel
	if (condition.TechLevel > 0)
	{
		if (pOwner->TechLevel >= condition.TechLevel)
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	// RequiredHouses & ForbiddenHouses
	if (condition.type == ConditionGroupType::SW || condition.type == ConditionGroupType::AEAttach || condition.type == ConditionGroupType::AEDiscard)
	{
		const auto OwnerBits = 1u << pOwner->Type->ArrayIndex;

		if (condition.RequiredHouses & OwnerBits)
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}

		if (!(condition.ForbiddenHouses & OwnerBits))
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	// Listed technos don't exist
	if (!condition.TechnosDontExist.empty())
	{
		if (!ConditionGroup::BatchCheckTechnoExist(pOwner, condition.TechnosDontExist, condition.TechnosDontExist_Houses, !condition.TechnosDontExist_Any, condition.TechnosDontExist_AllowLimboed))
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	// Listed technos exist
	if (!condition.TechnosExist.empty())
	{
		if (ConditionGroup::BatchCheckTechnoExist(pOwner, condition.TechnosExist, condition.TechnosExist_Houses, condition.TechnosExist_Any, condition.TechnosDontExist_AllowLimboed))
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	// if OnAnyCondition set to false, then all conditions have met in this step
	if (!condition.OnAnyCondition)
		return true;

	return false;
}

bool ConditionGroup::CheckTechnoConditions(TechnoClass* pTechno, const ConditionGroup condition)
{
	auto const pType = pTechno->GetTechnoType();
	auto const pOwner = pTechno->Owner;

	// Process house conditions
	if (pOwner)
	{
		if (ConditionGroup::CheckHouseConditions(pOwner, condition))
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	// Powered
	if (condition.PowerOn || condition.PowerOff)
	{
		bool isActive = !(pTechno->Deactivated || pTechno->IsUnderEMP());

		if (isActive && pTechno->WhatAmI() == AbstractType::Building)
		{
			auto const pBuilding = static_cast<BuildingClass const*>(pTechno);
			isActive = pBuilding->IsPowerOnline();
		}

		if (isActive)
		{
			if (condition.PowerOn && condition.OnAnyCondition)
				return true;

			if (condition.PowerOff && !condition.OnAnyCondition)
				return false;
		}
		else
		{
			if (condition.PowerOn && !condition.OnAnyCondition)
				return false;

			if (condition.PowerOff && condition.OnAnyCondition)
				return true;
		}
	}

	// Health
	if (condition.AbovePercent.isset())
	{
		if (pTechno->GetHealthPercentage() >= condition.AbovePercent.Get())
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	if (condition.BelowPercent.isset())
	{
		if (pTechno->GetHealthPercentage() <= condition.BelowPercent.Get())
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	// Ammo
	if (condition.AmmoExceed >= 0)
	{
		if (pType->Ammo > 0 && pTechno->Ammo >= condition.AmmoExceed)
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	if (condition.AmmoBelow >= 0)
	{
		if (pType->Ammo > 0 && pTechno->Ammo <= condition.AmmoBelow)
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	// Passengers
	if (condition.PassengersExceed >= 0)
	{
		if (pType->Passengers > 0 && pTechno->Passengers.NumPassengers >= condition.PassengersExceed)
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	if (condition.PassengersBelow >= 0)
	{
		if (pType->Passengers > 0 && pTechno->Passengers.NumPassengers <= condition.PassengersBelow)
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	// Move
	if (auto const pFoot = abstract_cast<FootClass*>(pTechno))
	{
		if (pFoot->Locomotor->Is_Really_Moving_Now())
		{
			if (condition.IsMoving && condition.OnAnyCondition)
				return true;

			if (condition.IsStationary && !condition.OnAnyCondition)
				return false;
		}
		else
		{
			if (condition.IsMoving && condition.OnAnyCondition)
				return false;

			if (condition.IsStationary && !condition.OnAnyCondition)
				return true;
		}
	}

	// Cloak
	if (condition.IsCloaked)
	{
		if (pTechno->CloakState == CloakState::Cloaked)
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	// Drain
	if (condition.IsDrained)
	{
		if (pTechno->DrainingMe)
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	// Shield
	auto const pExt = TechnoExt::ExtMap.Find(pTechno);

	if (auto const pShieldData = pExt->Shield.get())
	{
		if (pShieldData->IsActive())
		{
			if (condition.ShieldActive && condition.OnAnyCondition)
				return true;

			if (condition.ShieldInactive && !condition.OnAnyCondition)
				return false;

			if (condition.ShieldAbovePercent.isset() && pShieldData->GetHealthRatio() >= condition.ShieldAbovePercent)
			{
				if (condition.OnAnyCondition)
					return true;
			}
			else if (!condition.OnAnyCondition)
			{
				return false;
			}

			if (condition.ShieldBelowPercent.isset() && pShieldData->GetHealthRatio() <= condition.ShieldBelowPercent)
			{
				if (condition.OnAnyCondition)
					return true;
			}
			else if (!condition.OnAnyCondition)
			{
				return false;
			}
		}
		else
		{
			if (condition.ShieldActive && condition.OnAnyCondition)
				return false;

			if (condition.ShieldInactive && !condition.OnAnyCondition)
				return true;
		}
	}

	// if OnAnyCondition set to false, then all conditions have met in this step
	if (!condition.OnAnyCondition)
		return true;

	return false;
}

bool ConditionGroup::BatchCheckTechnoExist(HouseClass* pOwner, const ValueableVector<TechnoTypeClass*>& vTypes, AffectedHouse affectedHouse, bool any, bool allowLimbo)
{
	auto existSingleType = [pOwner, affectedHouse, allowLimbo](TechnoTypeClass* pType)
		{
			for (HouseClass* pHouse : *HouseClass::Array)
			{
				if (EnumFunctions::CanTargetHouse(affectedHouse, pOwner, pHouse)
					&& (allowLimbo ? HouseExt::ExtMap.Find(pHouse)->CountOwnedPresentAndLimboed(pType) > 0 : pHouse->CountOwnedAndPresent(pType) > 0))
					return true;
			}

			return false;
		};

	return any
		? std::any_of(vTypes.begin(), vTypes.end(), existSingleType)
		: std::all_of(vTypes.begin(), vTypes.end(), existSingleType);
}

bool ConditionGroup::CheckTechnoConditionsWithTimer(TechnoClass* pTechno, const ConditionGroup condition, CDTimerClass& Timer)
{
	// Process techno conditions
	if (ConditionGroup::CheckTechnoConditions(pTechno, condition))
	{
		if (condition.OnAnyCondition)
			return true;
	}
	else if (!condition.OnAnyCondition)
	{
		return false;
	}

	// Countdown ends
	if (condition.AfterDelay > 0)
	{
		if (!Timer.HasStarted())
		{
			Timer.Start(condition.AfterDelay);

			if (!condition.OnAnyCondition)
				return false;
		}
		else if (Timer.Completed())
		{
			if (condition.OnAnyCondition)
				return true;
		}
		else if (!condition.OnAnyCondition)
		{
			return false;
		}
	}

	// if OnAnyCondition set to false, then all conditions have met in this step
	if (!condition.OnAnyCondition)
		return true;

	return false;
}

ConditionGroup::ConditionGroup()
	: PowerOn { false }
	, PowerOff { false }
	, AmmoExceed { -1 }
	, AmmoBelow { -1 }
	, AfterDelay { 0 }
	, OwnedByPlayer { false }
	, OwnedByAI { false }
	, MoneyExceed { -1 }
	, MoneyBelow { -1 }
	, LowPower { false }
	, FullPower { false }
	, TechLevel { 0 }
	, RequiredHouses { 0xFFFFFFFFu }
	, ForbiddenHouses { 0u }
	, AbovePercent {}
	, BelowPercent {}
	, PassengersExceed { -1 }
	, PassengersBelow { -1 }
	, ShieldActive { false }
	, ShieldInactive { false }
	, ShieldAbovePercent {}
	, ShieldBelowPercent {}
	, IsMoving { false }
	, IsStationary { false }
	, IsCloaked { false }
	, IsDrained { false }
	, TechnosDontExist {}
	, TechnosDontExist_Any { false }
	, TechnosDontExist_AllowLimboed { false }
	, TechnosDontExist_Houses { AffectedHouse::Owner }
	, TechnosExist {}
	, TechnosExist_Any { true }
	, TechnosExist_AllowLimboed { false }
	, TechnosExist_Houses { AffectedHouse::Owner }
	, OnAnyCondition { true }
{
}

void ConditionGroup::ParseAEAttach(ConditionGroup& condition, CCINIClass* const pINI, INI_EX& exINI, const char* pSection)
{
	condition.AmmoExceed.Read(exINI, pSection, "AttachOn.AmmoExceed");
	condition.AmmoBelow.Read(exINI, pSection, "AttachOn.AmmoBelow");
	condition.OwnedByPlayer.Read(exINI, pSection, "AttachOn.OwnedByPlayer");
	condition.OwnedByAI.Read(exINI, pSection, "AttachOn.OwnedByAI");
	condition.LowPower.Read(exINI, pSection, "AttachOn.LowPower");
	condition.FullPower.Read(exINI, pSection, "AttachOn.FullPower");
	condition.RequiredHouses = pINI->ReadHouseTypesList(pSection, "AttachOn.RequiredHouses", condition.RequiredHouses);
	condition.ForbiddenHouses = pINI->ReadHouseTypesList(pSection, "AttachOn.ForbiddenHouses", condition.ForbiddenHouses);
	condition.AbovePercent.Read(exINI, pSection, "AttachOn.AbovePercent");
	condition.BelowPercent.Read(exINI, pSection, "AttachOn.BelowPercent");
	condition.PassengersExceed.Read(exINI, pSection, "AttachOn.PassengersExceed");
	condition.PassengersBelow.Read(exINI, pSection, "AttachOn.PassengersBelow");
	condition.TechnosDontExist.Read(exINI, pSection, "AttachOn.TechnosDontExist");
	condition.TechnosDontExist_Any.Read(exINI, pSection, "AttachOn.TechnosDontExist.Any");
	condition.TechnosDontExist_AllowLimboed.Read(exINI, pSection, "AttachOn.TechnosDontExist.AllowLimboed");
	condition.TechnosDontExist_Houses.Read(exINI, pSection, "AttachOn.TechnosDontExist.Houses");
	condition.TechnosExist.Read(exINI, pSection, "AttachOn.TechnosExist");
	condition.TechnosDontExist_Any.Read(exINI, pSection, "AttachOn.TechnosDontExist.Any");
	condition.TechnosExist_AllowLimboed.Read(exINI, pSection, "AttachOn.TechnosExist.AllowLimboed");
	condition.TechnosExist_Houses.Read(exINI, pSection, "AttachOn.TechnosExist.Houses");

	condition.OnAnyCondition = false;

	// Type of this condition group must be set
	condition.type = ConditionGroupType::AEAttach;
}

void ConditionGroup::ParseAEDiscard(ConditionGroup& condition, CCINIClass* const pINI, INI_EX& exINI, const char* pSection)
{
	condition.AmmoExceed.Read(exINI, pSection, "DiscardOn.AmmoExceed");
	condition.AmmoBelow.Read(exINI, pSection, "DiscardOn.AmmoBelow");
	condition.OwnedByPlayer.Read(exINI, pSection, "DiscardOn.OwnedByPlayer");
	condition.OwnedByAI.Read(exINI, pSection, "DiscardOn.OwnedByAI");
	condition.LowPower.Read(exINI, pSection, "DiscardOn.LowPower");
	condition.FullPower.Read(exINI, pSection, "DiscardOn.FullPower");
	condition.RequiredHouses = pINI->ReadHouseTypesList(pSection, "DiscardOn.RequiredHouses", condition.RequiredHouses);
	condition.ForbiddenHouses = pINI->ReadHouseTypesList(pSection, "DiscardOn.ForbiddenHouses", condition.ForbiddenHouses);
	condition.AbovePercent.Read(exINI, pSection, "DiscardOn.AbovePercent");
	condition.BelowPercent.Read(exINI, pSection, "DiscardOn.BelowPercent");
	condition.PassengersExceed.Read(exINI, pSection, "DiscardOn.PassengersExceed");
	condition.PassengersBelow.Read(exINI, pSection, "DiscardOn.PassengersBelow");
	condition.TechnosDontExist.Read(exINI, pSection, "DiscardOn.TechnosDontExist");
	condition.TechnosDontExist_Any.Read(exINI, pSection, "DiscardOn.TechnosDontExist.Any");
	condition.TechnosDontExist_AllowLimboed.Read(exINI, pSection, "DiscardOn.TechnosDontExist.AllowLimboed");
	condition.TechnosDontExist_Houses.Read(exINI, pSection, "DiscardOn.TechnosDontExist.Houses");
	condition.TechnosExist.Read(exINI, pSection, "DiscardOn.TechnosExist");
	condition.TechnosDontExist_Any.Read(exINI, pSection, "DiscardOn.TechnosDontExist.Any");
	condition.TechnosExist_AllowLimboed.Read(exINI, pSection, "DiscardOn.TechnosExist.AllowLimboed");
	condition.TechnosExist_Houses.Read(exINI, pSection, "DiscardOn.TechnosExist.Houses");

	// Type of this condition group must be set
	condition.type = ConditionGroupType::AEDiscard;
}

#pragma region(save/load)

template <typename T>
bool ConditionGroup::Serialize(T& stm)
{
	return stm
		.Process(this->PowerOn)
		.Process(this->PowerOff)
		.Process(this->AmmoExceed)
		.Process(this->AmmoBelow)
		.Process(this->AfterDelay)
		.Process(this->OwnedByPlayer)
		.Process(this->OwnedByAI)
		.Process(this->MoneyExceed)
		.Process(this->MoneyBelow)
		.Process(this->LowPower)
		.Process(this->FullPower)
		.Process(this->TechLevel)
		.Process(this->RequiredHouses)
		.Process(this->ForbiddenHouses)
		.Process(this->PassengersExceed)
		.Process(this->PassengersBelow)
		.Process(this->ShieldActive)
		.Process(this->ShieldInactive)
		.Process(this->ShieldAbovePercent)
		.Process(this->ShieldBelowPercent)
		.Process(this->IsMoving)
		.Process(this->IsStationary)
		.Process(this->IsCloaked)
		.Process(this->IsDrained)
		.Process(this->TechnosDontExist)
		.Process(this->TechnosDontExist_Any)
		.Process(this->TechnosDontExist_AllowLimboed)
		.Process(this->TechnosDontExist_Houses)
		.Process(this->TechnosExist)
		.Process(this->TechnosExist_Any)
		.Process(this->TechnosExist_AllowLimboed)
		.Process(this->TechnosExist_Houses)
		.Process(this->OnAnyCondition)
		.Success();
}

bool ConditionGroup::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool ConditionGroup::Save(PhobosStreamWriter& stm) const
{
	return const_cast<ConditionGroup*>(this)->Serialize(stm);
}

#pragma endregion(save/load)

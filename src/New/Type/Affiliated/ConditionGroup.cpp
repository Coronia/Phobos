#include "ConditionGroup.h"
#include <Ext/House/Body.h>
#include <Utilities/EnumFunctions.h>

bool ConditionGroup::CheckHouseConditions(HouseClass* pOwner)
{
	// Owning house
	if (pOwner->IsControlledByHuman())
	{
		if (this->OwnedByPlayer && this->OnAnyCondition)
			return true;

		if (this->OwnedByAI && !this->OnAnyCondition)
			return false;
	}
	else
	{
		if (this->OwnedByPlayer && !this->OnAnyCondition)
			return false;

		if (this->OwnedByAI && this->OnAnyCondition)
			return true;
	}

	// Money
	if (this->MoneyExceed >= 0)
	{
		if (pOwner->Available_Money() >= this->MoneyExceed)
		{
			if (this->OnAnyCondition)
				return true;
		}
		else if (!this->OnAnyCondition)
		{
			return false;
		}
	}

	if (this->MoneyBelow >= 0)
	{
		if (pOwner->Available_Money() <= this->MoneyBelow)
		{
			if (this->OnAnyCondition)
				return true;
		}
		else if (!this->OnAnyCondition)
		{
			return false;
		}
	}

	// Power
	if (pOwner->HasLowPower())
	{
		if (this->LowPower && this->OnAnyCondition)
			return true;

		if (this->FullPower && !this->OnAnyCondition)
			return false;
	}
	else if (pOwner->HasFullPower())
	{
		if (this->LowPower && !this->OnAnyCondition)
			return false;

		if (this->FullPower && this->OnAnyCondition)
			return true;
	}

	// TechLevel
	if (this->TechLevel > 0)
	{
		if (pOwner->TechLevel >= this->TechLevel)
		{
			if (this->OnAnyCondition)
				return true;
		}
		else if (!this->OnAnyCondition)
		{
			return false;
		}
	}

	// RequiredHouses & ForbiddenHouses
	if (this->type == ConditionGroupType::SW || this->type == ConditionGroupType::AEAttach || this->type == ConditionGroupType::AEDiscard)
	{
		const auto OwnerBits = 1u << pOwner->Type->ArrayIndex;

		if (this->RequiredHouses & OwnerBits)
		{
			if (this->OnAnyCondition)
				return true;
		}
		else if (!this->OnAnyCondition)
		{
			return false;
		}

		if (!(this->ForbiddenHouses & OwnerBits))
		{
			if (this->OnAnyCondition)
				return true;
		}
		else if (!this->OnAnyCondition)
		{
			return false;
		}
	}

	// Listed technos don't exist
	if (!this->TechnosDontExist.empty())
	{
		if (!ConditionGroup::BatchCheckTechnoExist(pOwner, this->TechnosDontExist, this->TechnosDontExist_Houses, !this->TechnosDontExist_Any, this->TechnosDontExist_AllowLimboed))
		{
			if (this->OnAnyCondition)
				return true;
		}
		else if (!this->OnAnyCondition)
		{
			return false;
		}
	}

	// Listed technos exist
	if (!this->TechnosExist.empty())
	{
		if (ConditionGroup::BatchCheckTechnoExist(pOwner, this->TechnosExist, this->TechnosExist_Houses, this->TechnosExist_Any, this->TechnosDontExist_AllowLimboed))
		{
			if (this->OnAnyCondition)
				return true;
		}
		else if (!this->OnAnyCondition)
		{
			return false;
		}
	}

	// if OnAnyCondition set to false, then all conditions have met in this step
	if (!this->OnAnyCondition)
		return true;

	return false;
}

bool ConditionGroup::CheckTechnoConditions(TechnoClass* pTechno)
{
	auto const pType = pTechno->GetTechnoType();
	auto const pOwner = pTechno->Owner;

	// Process house conditions
	if (pOwner)
	{
		if (this->CheckHouseConditions(pOwner))
		{
			if (this->OnAnyCondition)
				return true;
		}
		else if (!this->OnAnyCondition)
		{
			return false;
		}
	}

	// Powered
	if (this->PowerOn || this->PowerOff)
	{
		bool isActive = !(pTechno->Deactivated || pTechno->IsUnderEMP());

		if (isActive && pTechno->WhatAmI() == AbstractType::Building)
		{
			auto const pBuilding = static_cast<BuildingClass const*>(pTechno);
			isActive = pBuilding->IsPowerOnline();
		}

		if (isActive)
		{
			if (this->PowerOn && this->OnAnyCondition)
				return true;

			if (this->PowerOff && !this->OnAnyCondition)
				return false;
		}
		else
		{
			if (this->PowerOn && !this->OnAnyCondition)
				return false;

			if (this->PowerOff && this->OnAnyCondition)
				return true;
		}
	}

	// Health
	if (this->AbovePercent.isset())
	{
		if (pTechno->GetHealthPercentage() >= this->AbovePercent.Get())
		{
			if (this->OnAnyCondition)
				return true;
		}
		else if (!this->OnAnyCondition)
		{
			return false;
		}
	}

	if (this->BelowPercent.isset())
	{
		if (pTechno->GetHealthPercentage() <= this->BelowPercent.Get())
		{
			if (this->OnAnyCondition)
				return true;
		}
		else if (!this->OnAnyCondition)
		{
			return false;
		}
	}

	// Ammo
	if (this->AmmoExceed >= 0)
	{
		if (pType->Ammo > 0 && pTechno->Ammo >= this->AmmoExceed)
		{
			if (this->OnAnyCondition)
				return true;
		}
		else if (!this->OnAnyCondition)
		{
			return false;
		}
	}

	if (this->AmmoBelow >= 0)
	{
		if (pType->Ammo > 0 && pTechno->Ammo <= this->AmmoBelow)
		{
			if (this->OnAnyCondition)
				return true;
		}
		else if (!this->OnAnyCondition)
		{
			return false;
		}
	}

	// Passengers
	if (this->PassengersExceed >= 0)
	{
		if (pType->Passengers > 0 && pTechno->Passengers.NumPassengers >= this->PassengersExceed)
		{
			if (this->OnAnyCondition)
				return true;
		}
		else if (!this->OnAnyCondition)
		{
			return false;
		}
	}

	if (this->PassengersBelow >= 0)
	{
		if (pType->Passengers > 0 && pTechno->Passengers.NumPassengers <= this->PassengersBelow)
		{
			if (this->OnAnyCondition)
				return true;
		}
		else if (!this->OnAnyCondition)
		{
			return false;
		}
	}

	// Move
	if (auto const pFoot = abstract_cast<FootClass*>(pTechno))
	{
		if (pFoot->Locomotor->Is_Really_Moving_Now())
		{
			if (this->IsMoving && this->OnAnyCondition)
				return true;

			if (this->IsStationary && !this->OnAnyCondition)
				return false;
		}
		else
		{
			if (this->IsMoving && this->OnAnyCondition)
				return false;

			if (this->IsStationary && !this->OnAnyCondition)
				return true;
		}
	}

	// Cloak
	if (this->IsCloaked)
	{
		if (pTechno->CloakState == CloakState::Cloaked)
		{
			if (this->OnAnyCondition)
				return true;
		}
		else if (!this->OnAnyCondition)
		{
			return false;
		}
	}

	// Drain
	if (this->IsDrained)
	{
		if (pTechno->DrainingMe)
		{
			if (this->OnAnyCondition)
				return true;
		}
		else if (!this->OnAnyCondition)
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
			if (this->ShieldActive && this->OnAnyCondition)
				return true;

			if (this->ShieldInactive && !this->OnAnyCondition)
				return false;

			if (this->ShieldAbovePercent.isset() && pShieldData->GetHealthRatio() >= this->ShieldAbovePercent)
			{
				if (this->OnAnyCondition)
					return true;
			}
			else if (!this->OnAnyCondition)
			{
				return false;
			}

			if (this->ShieldBelowPercent.isset() && pShieldData->GetHealthRatio() <= this->ShieldBelowPercent)
			{
				if (this->OnAnyCondition)
					return true;
			}
			else if (!this->OnAnyCondition)
			{
				return false;
			}
		}
		else
		{
			if (this->ShieldActive && this->OnAnyCondition)
				return false;

			if (this->ShieldInactive && !this->OnAnyCondition)
				return true;
		}
	}

	// if OnAnyCondition set to false, then all conditions have met in this step
	if (!this->OnAnyCondition)
		return true;

	return false;
}

bool ConditionGroup::BatchCheckTechnoExist(HouseClass* pOwner, const ValueableVector<TechnoTypeClass*>& vTypes, AffectedHouse affectedHouse, bool any, bool allowLimbo) const
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

bool ConditionGroup::CheckTechnoConditionsWithTimer(TechnoClass* pTechno, CDTimerClass& Timer)
{
	// Process techno conditions
	if (this->CheckTechnoConditions(pTechno))
	{
		if (this->OnAnyCondition)
			return true;
	}
	else if (!this->OnAnyCondition)
	{
		return false;
	}

	// Countdown ends
	if (this->AfterDelay > 0)
	{
		if (!Timer.HasStarted())
		{
			Timer.Start(this->AfterDelay);

			if (!this->OnAnyCondition)
				return false;
		}
		else if (Timer.Completed())
		{
			if (this->OnAnyCondition)
				return true;
		}
		else if (!this->OnAnyCondition)
		{
			return false;
		}
	}

	// if OnAnyCondition set to false, then all conditions have met in this step
	if (!this->OnAnyCondition)
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

void ConditionGroup::ParseAEAttach(CCINIClass* const pINI, const char* pSection)
{
	INI_EX exINI(pINI);

	this->AmmoExceed.Read(exINI, pSection, "AttachOn.AmmoExceed");
	this->AmmoBelow.Read(exINI, pSection, "AttachOn.AmmoBelow");
	this->OwnedByPlayer.Read(exINI, pSection, "AttachOn.OwnedByPlayer");
	this->OwnedByAI.Read(exINI, pSection, "AttachOn.OwnedByAI");
	this->LowPower.Read(exINI, pSection, "AttachOn.LowPower");
	this->FullPower.Read(exINI, pSection, "AttachOn.FullPower");
	this->RequiredHouses = pINI->ReadHouseTypesList(pSection, "AttachOn.RequiredHouses", this->RequiredHouses);
	this->ForbiddenHouses = pINI->ReadHouseTypesList(pSection, "AttachOn.ForbiddenHouses", this->ForbiddenHouses);
	this->AbovePercent.Read(exINI, pSection, "AttachOn.AbovePercent");
	this->BelowPercent.Read(exINI, pSection, "AttachOn.BelowPercent");
	this->PassengersExceed.Read(exINI, pSection, "AttachOn.PassengersExceed");
	this->PassengersBelow.Read(exINI, pSection, "AttachOn.PassengersBelow");
	this->TechnosDontExist.Read(exINI, pSection, "AttachOn.TechnosDontExist");
	this->TechnosDontExist_Any.Read(exINI, pSection, "AttachOn.TechnosDontExist.Any");
	this->TechnosDontExist_AllowLimboed.Read(exINI, pSection, "AttachOn.TechnosDontExist.AllowLimboed");
	this->TechnosDontExist_Houses.Read(exINI, pSection, "AttachOn.TechnosDontExist.Houses");
	this->TechnosExist.Read(exINI, pSection, "AttachOn.TechnosExist");
	this->TechnosDontExist_Any.Read(exINI, pSection, "AttachOn.TechnosDontExist.Any");
	this->TechnosExist_AllowLimboed.Read(exINI, pSection, "AttachOn.TechnosExist.AllowLimboed");
	this->TechnosExist_Houses.Read(exINI, pSection, "AttachOn.TechnosExist.Houses");

	this->OnAnyCondition = false;

	// Type of this condition group must be set
	this->type = ConditionGroupType::AEAttach;
}

void ConditionGroup::ParseAEDiscard(CCINIClass* const pINI, const char* pSection)
{
	INI_EX exINI(pINI);

	this->AmmoExceed.Read(exINI, pSection, "DiscardOn.AmmoExceed");
	this->AmmoBelow.Read(exINI, pSection, "DiscardOn.AmmoBelow");
	this->OwnedByPlayer.Read(exINI, pSection, "DiscardOn.OwnedByPlayer");
	this->OwnedByAI.Read(exINI, pSection, "DiscardOn.OwnedByAI");
	this->LowPower.Read(exINI, pSection, "DiscardOn.LowPower");
	this->FullPower.Read(exINI, pSection, "DiscardOn.FullPower");
	this->RequiredHouses = pINI->ReadHouseTypesList(pSection, "DiscardOn.RequiredHouses", this->RequiredHouses);
	this->ForbiddenHouses = pINI->ReadHouseTypesList(pSection, "DiscardOn.ForbiddenHouses", this->ForbiddenHouses);
	this->AbovePercent.Read(exINI, pSection, "DiscardOn.AbovePercent");
	this->BelowPercent.Read(exINI, pSection, "DiscardOn.BelowPercent");
	this->PassengersExceed.Read(exINI, pSection, "DiscardOn.PassengersExceed");
	this->PassengersBelow.Read(exINI, pSection, "DiscardOn.PassengersBelow");
	this->TechnosDontExist.Read(exINI, pSection, "DiscardOn.TechnosDontExist");
	this->TechnosDontExist_Any.Read(exINI, pSection, "DiscardOn.TechnosDontExist.Any");
	this->TechnosDontExist_AllowLimboed.Read(exINI, pSection, "DiscardOn.TechnosDontExist.AllowLimboed");
	this->TechnosDontExist_Houses.Read(exINI, pSection, "DiscardOn.TechnosDontExist.Houses");
	this->TechnosExist.Read(exINI, pSection, "DiscardOn.TechnosExist");
	this->TechnosDontExist_Any.Read(exINI, pSection, "DiscardOn.TechnosDontExist.Any");
	this->TechnosExist_AllowLimboed.Read(exINI, pSection, "DiscardOn.TechnosExist.AllowLimboed");
	this->TechnosExist_Houses.Read(exINI, pSection, "DiscardOn.TechnosExist.Houses");

	// Type of this condition group must be set
	this->type = ConditionGroupType::AEDiscard;
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

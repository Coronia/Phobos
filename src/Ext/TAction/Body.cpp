#include "Body.h"

#include <SessionClass.h>
#include <MessageListClass.h>
#include <HouseClass.h>
#include <CRT.h>
#include <SuperWeaponTypeClass.h>
#include <SuperClass.h>
#include <Ext/SWType/Body.h>
#include <Utilities/SavegameDef.h>

#include <New/Entity/BannerClass.h>
#include <New/Type/BannerTypeClass.h>

#include <Ext/Scenario/Body.h>

//Static init
TActionExt::ExtContainer TActionExt::ExtMap;

// =============================
// load / save

template <typename T>
void TActionExt::ExtData::Serialize(T& Stm)
{
	//Stm;
}

void TActionExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TActionClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TActionExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TActionClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool TActionExt::Execute(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct const& location, bool& bHandled)
{
	bHandled = true;

	// Vanilla
	switch (pThis->ActionKind)
	{
	case TriggerAction::PlaySoundEffectRandom:
		return TActionExt::PlayAudioAtRandomWP(pThis, pHouse, pObject, pTrigger, location);
	default:
		break;
	};

	// Phobos
	switch (static_cast<PhobosTriggerAction>(pThis->ActionKind))
	{
	case PhobosTriggerAction::SaveGame:
		return TActionExt::SaveGame(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::EditVariable:
		return TActionExt::EditVariable(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::GenerateRandomNumber:
		return TActionExt::GenerateRandomNumber(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::PrintVariableValue:
		return TActionExt::PrintVariableValue(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::BinaryOperation:
		return TActionExt::BinaryOperation(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::RunSuperWeaponAtLocation:
		return TActionExt::RunSuperWeaponAtLocation(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::RunSuperWeaponAtWaypoint:
		return TActionExt::RunSuperWeaponAtWaypoint(pThis, pHouse, pObject, pTrigger, location);

	case PhobosTriggerAction::ToggleMCVRedeploy:
		return TActionExt::ToggleMCVRedeploy(pThis, pHouse, pObject, pTrigger, location);

	case PhobosTriggerAction::CreateBannerGlobal:
		return TActionExt::CreateBannerGlobal(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::CreateBannerLocal:
		return TActionExt::CreateBannerLocal(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::DeleteBanner:
		return TActionExt::DeleteBanner(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::ResetBanner:
		return TActionExt::ResetBanner(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::PauseBanner:
		return TActionExt::PauseBanner(pThis, pHouse, pObject, pTrigger, location);

	default:
		bHandled = false;
		return true;
	}
}

bool TActionExt::PlayAudioAtRandomWP(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	std::vector<int> waypoints;
	waypoints.reserve(ScenarioExt::Global()->Waypoints.size());

	auto const pScen = ScenarioClass::Instance();

	for (auto pair : ScenarioExt::Global()->Waypoints)
		if (pScen->IsDefinedWaypoint(pair.first))
			waypoints.push_back(pair.first);

	if (waypoints.size() > 0)
	{
		auto const index = pScen->Random.RandomRanged(0, waypoints.size() - 1);
		auto const luckyWP = waypoints[index];
		auto const cell = pScen->GetWaypointCoords(luckyWP);
		auto const coords = CellClass::Cell2Coord(cell);
		VocClass::PlayIndexAtPos(pThis->Value, coords);
	}

	return true;
}

bool TActionExt::SaveGame(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (SessionClass::IsSingleplayer())
	{
		auto PrintMessage = [](const wchar_t* pMessage)
		{
			MessageListClass::Instance->PrintMessage(
				pMessage,
				RulesClass::Instance->MessageDelay,
				HouseClass::CurrentPlayer->ColorSchemeIndex,
				true
			);
		};

		char fName[0x80];

		SYSTEMTIME time;
		GetLocalTime(&time);

		_snprintf_s(fName, 0x7F, "Map.%04u%02u%02u-%02u%02u%02u-%05u.sav",
			time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

		PrintMessage(StringTable::LoadString(GameStrings::TXT_SAVING_GAME));

		wchar_t fDescription[0x80] = { 0 };
		wcscpy_s(fDescription, ScenarioClass::Instance->UINameLoaded);
		wcscat_s(fDescription, L" - ");
		wcscat_s(fDescription, StringTable::LoadString(pThis->Text));

		if (ScenarioClass::Instance->SaveGame(fName, fDescription))
			PrintMessage(StringTable::LoadString(GameStrings::TXT_GAME_WAS_SAVED));
		else
			PrintMessage(StringTable::LoadString(GameStrings::TXT_ERROR_SAVING_GAME));
	}

	return true;
}

bool TActionExt::EditVariable(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	// Variable Index
	// holds by pThis->Value

	// Operations:
	// 0 : set value - operator=
	// 1 : add value - operator+
	// 2 : minus value - operator-
	// 3 : multiply value - operator*
	// 4 : divide value - operator/
	// 5 : mod value - operator%
	// 6 : <<
	// 7 : >>
	// 8 : ~ (no second param being used)
	// 9 : ^
	// 10 : |
	// 11 : &
	// holds by pThis->Param3

	// Params:
	// The second value
	// holds by pThis->Param4

	// Global Variable or Local
	// 0 for local and 1 for global
	// holds by pThis->Param5

	// uses !pThis->Param5 to ensure Param5 is 0 or 1
	auto& variables = ScenarioExt::Global()->Variables[pThis->Param5 != 0];
	auto itr = variables.find(pThis->Value);
	if (itr != variables.end())
	{
		auto& nCurrentValue = itr->second.Value;
		// variable being found
		switch (pThis->Param3)
		{
		case 0: { nCurrentValue = pThis->Param4; break; }
		case 1: { nCurrentValue += pThis->Param4; break; }
		case 2: { nCurrentValue -= pThis->Param4; break; }
		case 3: { nCurrentValue *= pThis->Param4; break; }
		case 4: { nCurrentValue /= pThis->Param4; break; }
		case 5: { nCurrentValue %= pThis->Param4; break; }
		case 6: { nCurrentValue <<= pThis->Param4; break; }
		case 7: { nCurrentValue >>= pThis->Param4; break; }
		case 8: { nCurrentValue = ~nCurrentValue; break; }
		case 9: { nCurrentValue ^= pThis->Param4; break; }
		case 10: { nCurrentValue |= pThis->Param4; break; }
		case 11: { nCurrentValue &= pThis->Param4; break; }
		default:
			return true;
		}

		if (!pThis->Param5)
			TagClass::NotifyLocalChanged(pThis->Value);
		else
			TagClass::NotifyGlobalChanged(pThis->Value);
	}
	return true;
}

bool TActionExt::GenerateRandomNumber(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto& variables = ScenarioExt::Global()->Variables[pThis->Param5 != 0];
	auto itr = variables.find(pThis->Value);
	if (itr != variables.end())
	{
		itr->second.Value = ScenarioClass::Instance->Random.RandomRanged(pThis->Param3, pThis->Param4);
		if (!pThis->Param5)
			TagClass::NotifyLocalChanged(pThis->Value);
		else
			TagClass::NotifyGlobalChanged(pThis->Value);
	}

	return true;
}

bool TActionExt::PrintVariableValue(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto& variables = ScenarioExt::Global()->Variables[pThis->Param3 != 0];
	auto itr = variables.find(pThis->Value);
	if (itr != variables.end())
	{
		CRT::swprintf(Phobos::wideBuffer, L"%d", itr->second.Value);
		MessageListClass::Instance->PrintMessage(Phobos::wideBuffer);
	}

	return true;
}

bool TActionExt::BinaryOperation(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto& variables1 = ScenarioExt::Global()->Variables[pThis->Param5 != 0];
	auto itr1 = variables1.find(pThis->Value);
	auto& variables2 = ScenarioExt::Global()->Variables[pThis->Param6 != 0];
	auto itr2 = variables2.find(pThis->Param4);

	if (itr1 != variables1.end() && itr2 != variables2.end())
	{
		auto& nCurrentValue = itr1->second.Value;
		auto& nOptValue = itr2->second.Value;
		switch (pThis->Param3)
		{
		case 0: { nCurrentValue = nOptValue; break; }
		case 1: { nCurrentValue += nOptValue; break; }
		case 2: { nCurrentValue -= nOptValue; break; }
		case 3: { nCurrentValue *= nOptValue; break; }
		case 4: { nCurrentValue /= nOptValue; break; }
		case 5: { nCurrentValue %= nOptValue; break; }
		case 6: { nCurrentValue <<= nOptValue; break; }
		case 7: { nCurrentValue >>= nOptValue; break; }
		case 8: { nCurrentValue = nOptValue; break; }
		case 9: { nCurrentValue ^= nOptValue; break; }
		case 10: { nCurrentValue |= nOptValue; break; }
		case 11: { nCurrentValue &= nOptValue; break; }
		default:
			return true;
		}

		if (!pThis->Param5)
			TagClass::NotifyLocalChanged(pThis->Value);
		else
			TagClass::NotifyGlobalChanged(pThis->Value);
	}
	return true;
}

bool TActionExt::RunSuperWeaponAtLocation(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (!pThis)
		return true;

	TActionExt::RunSuperWeaponAt(pThis, pThis->Param5, pThis->Param6);

	return true;
}

bool TActionExt::RunSuperWeaponAtWaypoint(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (!pThis)
		return true;

	auto& waypoints = ScenarioExt::Global()->Waypoints;
	int nWaypoint = pThis->Param5;

	// Check if is a valid Waypoint
	if (nWaypoint >= 0 && waypoints.find(nWaypoint) != waypoints.end() && waypoints[nWaypoint].X && waypoints[nWaypoint].Y)
	{
		auto const selectedWP = waypoints[nWaypoint];
		TActionExt::RunSuperWeaponAt(pThis, selectedWP.X, selectedWP.Y);
	}

	return true;
}

bool TActionExt::RunSuperWeaponAt(TActionClass* pThis, int X, int Y)
{
	if (SuperWeaponTypeClass::Array->Count > 0)
	{
		int swIdx = pThis->Param3;
		int houseIdx = -1;
		std::vector<int> housesListIdx;
		CellStruct targetLocation = { (short)X, (short)Y };

		do
		{
			if (X < 0)
				targetLocation.X = (short)ScenarioClass::Instance->Random.RandomRanged(0, MapClass::Instance->MapCoordBounds.Right);

			if (Y < 0)
				targetLocation.Y = (short)ScenarioClass::Instance->Random.RandomRanged(0, MapClass::Instance->MapCoordBounds.Bottom);
		}
		while (!MapClass::Instance->IsWithinUsableArea(targetLocation, false));

		// Only valid House indexes
		if ((pThis->Param4 >= HouseClass::Array->Count
			&& pThis->Param4 < HouseClass::PlayerAtA)
			|| pThis->Param4 > (HouseClass::PlayerAtA + HouseClass::Array->Count - 3))
		{
			return true;
		}

		switch (pThis->Param4)
		{
		case HouseClass::PlayerAtA:
			houseIdx = 0;
			break;

		case HouseClass::PlayerAtB:
			houseIdx = 1;
			break;

		case HouseClass::PlayerAtC:
			houseIdx = 2;
			break;

		case HouseClass::PlayerAtD:
			houseIdx = 3;
			break;

		case HouseClass::PlayerAtE:
			houseIdx = 4;
			break;

		case HouseClass::PlayerAtF:
			houseIdx = 5;
			break;

		case HouseClass::PlayerAtG:
			houseIdx = 6;
			break;

		case HouseClass::PlayerAtH:
			houseIdx = 7;
			break;

		case -1:
			// Random non-neutral
			for (auto pHouse : *HouseClass::Array)
			{
				if (!pHouse->Defeated
					&& !pHouse->IsObserver()
					&& !pHouse->Type->MultiplayPassive)
				{
					housesListIdx.push_back(pHouse->ArrayIndex);
				}
			}

			if (housesListIdx.size() > 0)
				houseIdx = housesListIdx.at(ScenarioClass::Instance->Random.RandomRanged(0, housesListIdx.size() - 1));
			else
				return true;

			break;

		case -2:
			// Find first Neutral
			for (auto pHouseNeutral : *HouseClass::Array)
			{
				if (pHouseNeutral->IsNeutral())
				{
					houseIdx = pHouseNeutral->ArrayIndex;
					break;
				}
			}

			if (houseIdx < 0)
				return true;

			break;

		case -3:
			// Random Human Player
			for (auto pHouse : *HouseClass::Array)
			{
				if (pHouse->IsControlledByHuman()
					&& !pHouse->Defeated
					&& !pHouse->IsObserver())
				{
					housesListIdx.push_back(pHouse->ArrayIndex);
				}
			}

			if (housesListIdx.size() > 0)
				houseIdx = housesListIdx.at(ScenarioClass::Instance->Random.RandomRanged(0, housesListIdx.size() - 1));
			else
				return true;

			break;

		default:
			if (pThis->Param4 >= 0)
				houseIdx = pThis->Param4;
			else
				return true;

			break;
		}

		if (HouseClass* pHouse = HouseClass::Array->GetItem(houseIdx))
		{
			if (auto const pSuper = pHouse->Supers.GetItem(swIdx))
			{
				int oldstart = pSuper->RechargeTimer.StartTime;
				int oldleft = pSuper->RechargeTimer.TimeLeft;
				pSuper->SetReadiness(true);
				pSuper->Launch(targetLocation, false);
				pSuper->Reset();
				pSuper->RechargeTimer.StartTime = oldstart;
				pSuper->RechargeTimer.TimeLeft = oldleft;
			}
		}
	}

	return true;
}

bool CreateOrReplaceBanner(TActionClass* pTAction, bool isGlobal)
{
	const auto pExt = TActionExt::ExtMap.Find(pTAction);
	BannerTypeClass* pBannerType = BannerTypeClass::Find(pExt->Parm3.data());

	if (!pBannerType)
	{
		int idx = atoi(pExt->Parm3.data());
		pBannerType = BannerTypeClass::Array[idx].get();
	}

	if (!pBannerType)
		return false;

	bool found = false;
	int id = atoi(pTAction->Text);
	if (id < 0)
		return false;

	for (auto& pBanner : BannerClass::Array)
	{
		if (!pBanner)
			continue;

		if (!pBanner->Type)
			continue;

		if (pBanner->Id != id)
			continue;

		if (pBanner->Type != pBannerType)
		{
			pBanner->Type = pBannerType;
			pBanner->Initilize = false;

			if (pBannerType->Type == BannerType::SHP)
			{
				if (pBannerType->Content_SHP_Play.Get())
					pBanner->Rate = 0;
				else
					pBanner->Rate = -1;

				if (pBannerType->Content_SHP_Remove.Get() && pBannerType->Content_SHP.isset())
				{
					int frame = pBannerType->Content_SHP.Get()->Frames - 1;
					pBanner->FrameIdx = pBannerType->Content_SHP_Frame.Get(frame);
				}
				else
					pBanner->FrameIdx = pBannerType->Content_SHP_Frame.Get(0);

				if (pBannerType->Content_SHP_FadeIn.Get())
					pBanner->FadeLevel = 4;
				else if (pBannerType->Content_SHP_FadeOut.Get())
					pBanner->FadeLevel = 0;
				else
					pBanner->FadeLevel = pBannerType->Content_SHP_Transparency.Get();

				if (pBannerType->Content_SHP_DrawInShroud.Get())
				{
					if (pBannerType->Content_PAL.GetConvert())
						pBanner->PAL = pBannerType->Content_PAL.GetConvert();
					else
						pBanner->PAL = FileSystem::PALETTE_PAL;
				}
				else
				{
					const auto pPAL = FileSystem::LoadPALFile(pBannerType->Content_PAL_Name.data(), DSurface::Composite);
					if (pPAL)
						pBanner->PAL = pPAL;
					else
						pBanner->PAL = FileSystem::PALETTE_PAL;
				}
			}
			else if (pBannerType->Type == BannerType::CSF)
			{
				pBanner->Length = 0;
			}

			if (pBannerType->Duration.Get() > 0)
				pBanner->Duration = pBannerType->Duration.Get();
			else
				pBanner->Duration = -1;
		}
		else
		{
			if (pBannerType->Duration.Get() > 0 && pBanner->Duration < pBannerType->Duration.Get())
			{
				pBanner->Duration = pBannerType->Duration.Get();
			}
		}

		if (pBanner->Position.X != pTAction->Param4 || pBanner->Position.Y != pTAction->Param5)
			pBanner->Position = CoordStruct { pTAction->Param4, pTAction->Param5, 0 };

		if (pBanner->Variable != pTAction->Param6)
			pBanner->Variable = pTAction->Param6;

		if (pBanner->IsGlobalVariable != isGlobal)
			pBanner->IsGlobalVariable = isGlobal;

		found = true;
		break;
	}

	if (!found)
	{
		BannerClass::Create(pBannerType, id,
			CoordStruct { pTAction->Param4, pTAction->Param5, 0 }, pTAction->Param6, isGlobal);

		return true;
	}

	return false;
}

void SortBanners(int start, int end)
{
	if (BannerClass::Array.size() <= 0)
		return;

	// just a quicksort
	if (start >= end)
		return;

	const auto& pBanner = BannerClass::Array[start];
	int pivotId = pBanner->Id;

	int count = 0;
	for (int i = start + 1; i <= end; i++)
	{
		const auto& pBanner2 = BannerClass::Array[i];

		if (pBanner2->Id <= pivotId)
			count++;
	}

	int pivot = start + count;
	std::swap(BannerClass::Array[pivot], BannerClass::Array[start]);

	int i = start, j = end;
	while (i < pivot && j > pivot)
	{
		const auto& pBannerA = BannerClass::Array[i];
		const auto& pBannerB = BannerClass::Array[j];

		while (pBannerA->Id <= pivotId)
			i++;

		while (pBannerB->Id > pivotId)
			j--;

		if (i < pivot && j > pivot)
			std::swap(BannerClass::Array[i++], BannerClass::Array[j--]);
	}

	SortBanners(start, pivot - 1);
	SortBanners(pivot + 1, end);
}

bool TActionExt::CreateBannerGlobal(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (CreateOrReplaceBanner(pThis, true))
		SortBanners(0, BannerClass::Array.size() - 1);

	return true;
}

bool TActionExt::CreateBannerLocal(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (CreateOrReplaceBanner(pThis, false))
		SortBanners(0, BannerClass::Array.size() - 1);

	return true;
}

bool TActionExt::DeleteBanner(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (BannerClass::Array.size() <= 0)
		return true;

	int id = atoi(pThis->Text);
	if (id < 0)
		return true;

	for (auto& pBanner : BannerClass::Array)
	{
		if (!pBanner)
			continue;

		if (!pBanner->Type)
			continue;

		if (pBanner->Id != id)
			continue;

		if (pBanner->Type->Type == BannerType::SHP &&
			pBanner->Type->Content_SHP_FadeIn.Get() &&
			pBanner->Type->Content_SHP_FadeOut.Get())
		{
			pBanner->Initilize = true;
			pBanner->Duration = 0;
		}
		else
		{
			BannerClass::Delete(pBanner);
		}

		break;
	}

	return true;
}

bool TActionExt::ResetBanner(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	int id = atoi(pThis->Text);
	if (id < 0)
		return true;

	for (auto& pBanner : BannerClass::Array)
	{
		if (!pBanner)
			continue;

		if (!pBanner->Type)
			continue;

		if (pBanner->Id != id)
			continue;

		pBanner->Initilize = false;

		if (pBanner->Type->Type == BannerType::SHP)
		{
			if (pBanner->Type->Content_SHP_Play.Get())
				pBanner->Rate = 0;
			else
				pBanner->Rate = -1;

			if (pBanner->Type->Content_SHP_Remove.Get() && pBanner->Type->Content_SHP.isset())
			{
				int frame = pBanner->Type->Content_SHP.Get()->Frames - 1;
				pBanner->FrameIdx = pBanner->Type->Content_SHP_Frame.Get(frame);
			}
			else
				pBanner->FrameIdx = pBanner->Type->Content_SHP_Frame.Get(0);

			if (pBanner->Type->Content_SHP_FadeIn.Get())
				pBanner->FadeLevel = 4;
			else if (pBanner->Type->Content_SHP_FadeOut.Get())
				pBanner->FadeLevel = 0;
			else
				pBanner->FadeLevel = pBanner->Type->Content_SHP_Transparency.Get();
		}
		else if (pBanner->Type->Type == BannerType::CSF)
		{
			pBanner->Length = 0;
		}

		if (pBanner->Type->Duration.Get() > 0)
			pBanner->Duration = pBanner->Type->Duration.Get();
		else
			pBanner->Duration = -1;

		break;
	}

	return true;
}

bool TActionExt::PauseBanner(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	int id = atoi(pThis->Text);
	if (id < 0)
		return true;

	for (auto& pBanner : BannerClass::Array)
	{
		if (!pBanner)
			continue;

		if (!pBanner->Type)
			continue;

		if (pBanner->Id != id)
			continue;

		pBanner->InPause = pThis->Param3 == 1 ? true : false;
		break;
	}

	return true;
}

bool TActionExt::ToggleMCVRedeploy(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	GameModeOptionsClass::Instance->MCVRedeploy = pThis->Param3 != 0;
	return true;
}


// =============================
// container

TActionExt::ExtContainer::ExtContainer() : Container("TActionClass") { }

TActionExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

#ifdef MAKE_GAME_SLOWER_FOR_NO_REASON
DEFINE_HOOK(0x6DD176, TActionClass_CTOR, 0x5)
{
	GET(TActionClass*, pItem, ESI);

	TActionExt::ExtMap.TryAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x6E4761, TActionClass_SDDTOR, 0x6)
{
	GET(TActionClass*, pItem, ESI);

	TActionExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6E3E30, TActionClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x6E3DB0, TActionClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(TActionClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TActionExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x6E3E29, TActionClass_Load_Suffix, 0x4)
{
	TActionExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x6E3E4A, TActionClass_Save_Suffix, 0x3)
{
	TActionExt::ExtMap.SaveStatic();
	return 0;
}
#endif

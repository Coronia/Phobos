#include <UnitClass.h>
#include <BuildingClass.h>
#include <ScenarioClass.h>
#include <HouseClass.h>

#include "Body.h"
#include "../BulletType/Body.h"
#include "../Techno/Body.h"
#include "../../ExtraHeaders/Matrix3D.h"

DEFINE_HOOK(6F64A9, HealthBar_Hide, 5)
{
	GET(TechnoClass*, pThis, ECX);
	auto pTypeData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if (pTypeData->HealthBar_Hide) {
		return 0x6F6AB6;
	}
	return 0;
}

DEFINE_HOOK(739956, UnitClass_Deploy_Transfer, 6)
{
	GET(UnitClass*, pUnit, EBP);
	GET(BuildingClass*, pStructure, EBX);

	// Vehicle-to-building deployer targeting
	auto pTypeData = TechnoTypeExt::ExtMap.Find(pStructure->GetTechnoType());
	if (pTypeData->Deployed_RememberTarget)
	{ // && pUnit->Target > 0)
		pStructure->Target = pUnit->Target;
	}

	return 0;
}

DEFINE_HOOK(6F9E50, TechnoClass_Update, 5)
{
	GET(TechnoClass*, pThis, ECX);

	// MindControlRangeLimit
	if (auto Capturer = pThis->MindControlledBy) {
		auto pCapturerExt = TechnoTypeExt::ExtMap.Find(Capturer->GetTechnoType());
		if (pCapturerExt->MindControlRangeLimit > 0 && pThis->DistanceFrom(Capturer) > pCapturerExt->MindControlRangeLimit * 256.0) {
			Capturer->CaptureManager->FreeUnit(pThis);
			if (!pThis->IsHumanControlled) {
				pThis->QueueMission(Mission::Hunt, 0);
			};
		}
	}

	// BuildingDeployerTargeting
	if (pThis->WhatAmI() == AbstractType::Building) {
		auto pTypeData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

		// Prevent target loss when vehicles are deployed into buildings.
		if (pTypeData->Deployed_RememberTarget)
		{
			auto currentMission = pThis->CurrentMission;
			// With this the vehicle will not forget who is the target until the deploy process finish
			if (pThis->Target > 0 &&
				currentMission != Mission::Construction &&
				currentMission != Mission::Guard &&
				currentMission != Mission::Attack &&
				currentMission != Mission::Selling
				) {
				pThis->QueueMission(Mission::Construction, 0);
				pThis->LastTarget = pThis->Target;
			}
			else if (pThis->Target == 0 && currentMission == Mission::Construction) {
				// Just when the deployment into structure ended the vehicle forgot the target. Just attack the original target.
				pThis->Target = pThis->LastTarget;
				pThis->QueueMission(Mission::Attack, 0);
			}
		}
	}

	// Interceptor
	auto pData = TechnoExt::ExtMap.Find(pThis);
	auto pTypeData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if (pTypeData->Interceptor && !pThis->Target)
	{
		for (auto const& pBullet : *BulletClass::Array) {
			if (auto pBulletTypeData = BulletTypeExt::ExtMap.Find(pBullet->Type)) {
				if (!pBulletTypeData->Interceptable)
					continue;
			}

			const double guardRange = pThis->Veterancy.IsElite() ?
				pTypeData->Interceptor_EliteGuardRange * 256 : pTypeData->Interceptor_GuardRange * 256;

			if (pBullet->Location.DistanceFrom(pThis->Location) > guardRange)
				continue;

			if (pBullet->Location.DistanceFrom(pBullet->TargetCoords) >
				double(ScenarioClass::Instance->Random.RandomRanged(128, (int)guardRange / 10)) * 10)
				continue;

			if (auto pTarget = abstract_cast<TechnoClass*>(pBullet->Target)) {
				if (pThis->Owner->IsAlliedWith(pTarget)) {
					pThis->SetTarget(pBullet);
					pData->InterceptedBullet = pBullet;
					break;
				}
			}
		}
	}

	return 0;
}

DEFINE_HOOK(6F3C56, TechnoClass_Transform_6F3AD0_TurretMultiOffset, 0)
{
	LEA_STACK(Matrix3D*, mtx, 0xD8 - 0x90);
	GET(TechnoTypeClass*, technoType, EDX);

	float x = static_cast<float>(technoType->TurretOffset);
	mtx->Translate(x, 0.0f, 0.0f);

	return 0x6F3C6D;
}

DEFINE_HOOK(6F3E6E, FootClass_firecoord_6F3D60_TurretMultiOffset, 0)
{
	LEA_STACK(Matrix3D*, mtx, 0xCC - 0x90);
	GET(TechnoTypeClass*, technoType, EBP);

	float x = static_cast<float>(technoType->TurretOffset);
	mtx->Translate(x, 0.0f, 0.0f);

	return 0x6F3E85;
}

DEFINE_HOOK(73B780, UnitClass_DrawVXL_MultiTurretOffset0, 0)
{
	GET(TechnoTypeClass*, technoType, EAX);

	if (!technoType->TurretOffset) {
		return 0x73B78A;
	}

	return 0x73B790;
}

DEFINE_HOOK(73BA4C, UnitClass_DrawVXL_MultiTurretOffset1, 0)
{
	LEA_STACK(Matrix3D*, mtx, 0x1D0 - 0x13C);
	GET(TechnoTypeClass*, technoType, EBX);

	double& factor = *reinterpret_cast<double*>(0xB1D008);

	float x = static_cast<float>(technoType->TurretOffset * factor);
	mtx->Translate(x, 0.0f, 0.0f);

	return 0x73BA68;
}
//73C88A

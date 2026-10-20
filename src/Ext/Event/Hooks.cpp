#include <Phobos.h>

#include <Helpers/Macro.h>
#include <Utilities/Debug.h>

static int PlanningNodeConfig = 0;
static int PlanningNodeBranchCap = 0x7FFFFFFF;
static int PlanningNodeWarnAt = 0x7FFFFFFF;
static bool PlanningNodeInitialized = false;
static bool PlanningNodeUnlimitedMode = true;

static void EnsurePlanningNodeInit()
{
	if (PlanningNodeInitialized)
		return;

	PlanningNodeInitialized = true;
	PlanningNodeConfig = Phobos::Config::PlanningNodeCap;

	if (PlanningNodeConfig < 0)
		PlanningNodeConfig = 0;

	if (PlanningNodeConfig > 4096)
		PlanningNodeConfig = 4096;

	if (PlanningNodeConfig > 0)
	{
		PlanningNodeBranchCap = PlanningNodeConfig;
		PlanningNodeWarnAt = PlanningNodeConfig - 1;
		PlanningNodeUnlimitedMode = false;
	}
}

// ============================================================================
// Hook 1: branch cap @ 0x6384DC
//  Vanilla: cmp $0x10, %eax; jne 0x638477
//  Semantics: eax < cap -> create branch; eax >= cap -> reject
//
//  Disassembly note: eax is only consumed by `cmp`. In unlimited mode we can
//  short-circuit without reading EAX at all.
// ============================================================================

DEFINE_HOOK(0x6384DC, WaypointCap_BranchCheck, 0x5)
{
	EnsurePlanningNodeInit();

	if (PlanningNodeUnlimitedMode)
		return 0x638477;   // unlimited: skip EAX read, create branch directly

	GET(const int, branchCount, EAX);

	return branchCount < PlanningNodeBranchCap ? 0x638477 : 0x6384E1;
}

// ============================================================================
// Hook 2: "about to reach cap" prompt @ 0x6383B4
//  Vanilla: cmpl $0xF, 0x14(%ebp); jne 0x6383C4
//  Semantics: token == warnAt -> store state 2 (UI plays cap sound); else continue
//
//  Disassembly note: ebp is only consumed by `[ebp+0x14]`. In unlimited mode
//  warnAt is unreachable, so we can skip the EBP read entirely.
// ============================================================================

DEFINE_HOOK(0x6383B4, WaypointCap_WarnCheck, 0x6)
{
	EnsurePlanningNodeInit();

	if (PlanningNodeUnlimitedMode)
		return 0x6383C4;   // unlimited: skip EBP read, continue directly

	GET(const int, value, EBP);

	const int token = *reinterpret_cast<const int*>(value + 0x14);
	return token == PlanningNodeWarnAt ? 0x6383BA : 0x6383C4;
}

// ============================================================================
// Hook 3: pre-order total check @ 0x6391A7
//  Vanilla: cmp $0x80, %ebp; jle 0x639203
//  Semantics: always passes (overflow is guarded on the dispatch side)
//
//  Disassembly note: at the hook site the stack still holds [ebp_orig, ebx_orig]
//  waiting to be popped. A naked tail stub is required to pop them first and then
//  jump straight to the final target, bypassing the flag-sensitive `jle`.
// ============================================================================

static __declspec(naked) void PlanningNodeOrderContinue()
{
	__asm
	{
		pop ebp
		pop ebx
		push 0x639203
		ret
	}
}

DEFINE_HOOK(0x6391A7, WaypointCap_OrderCheck, 0x6)
{
	EnsurePlanningNodeInit();
	return reinterpret_cast<DWORD>(PlanningNodeOrderContinue);
}

#include "ToggleSWSidebar.h"
#include <HouseClass.h>

#include <Utilities/GeneralUtils.h>
#include <Ext/Sidebar/SWSidebar/SWSidebarClass.h>

const char* ToggleSWSidebarCommandClass::GetName() const
{
	return "Toggle Super Weapon Sidebar";
}

const wchar_t* ToggleSWSidebarCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_TOGGLE_SW_SIDEBAR", L"Toggle Super Weapon Sidebar");
}

const wchar_t* ToggleSWSidebarCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* ToggleSWSidebarCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_TOGGLE_SW_SIDEBAR_DESC", L"Toggle the Super Weapon Sidebar.");
}

void ToggleSWSidebarCommandClass::Execute(WWKey eInput) const
{
	ToggleSWButtonClass::SwitchSidebar();

	if (SWSidebarClass::Instance.CurrentColumn)
		MouseClass::Instance.UpdateCursor(MouseCursorType::Default, false);
}

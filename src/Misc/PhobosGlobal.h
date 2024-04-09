#pragma once
#include <map>

class PhobosGlobal
{
public:
	std::multimap<int, TechnoClass*, std::greater<int>> Techno_HugeBar;

	bool Save(PhobosStreamWriter& stm);
	bool Load(PhobosStreamReader& stm);

	static bool SaveGlobals(PhobosStreamWriter& stm);
	static bool LoadGlobals(PhobosStreamReader& stm);

	inline static PhobosGlobal* Global()
	{
		return &GlobalObject;
	}

	PhobosGlobal() :
		Techno_HugeBar()
	{
	}

	~PhobosGlobal() = default;

	static void PointerGotInvalid(void* ptr, bool removed);
	static void Clear();
	void InvalidatePointer(void* ptr, bool removed);
	void Reset();

private:
	template <typename T>
	bool Serialize(T& stm);

	static PhobosGlobal GlobalObject;
};

#pragma once
#include <windows.h>
#include <cstdint>
#include <cstddef>
#include "../THashMap/THashMap.hpp"
#include "../IConfigVar/IConfigVar.hpp"

// Config::CConfigVarRegistry: FNV-1a(key, group) -> IConfigVar*, guarded by its own lock.
#pragma pack(push, 1)
struct CConfigVarRegistry
{
	THashMap<uint32_t, IConfigVar *> vars;
	CRITICAL_SECTION lock;
};
#pragma pack(pop)

static_assert(offsetof(CConfigVarRegistry, lock) == 0x30);

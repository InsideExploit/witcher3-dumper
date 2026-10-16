#pragma once
#include <cstdint>
#include <cstddef>
#include "../IRTTIType/IRTTIType.hpp"
#include "../TDynArray/TDynArray.hpp"
#include "../THashMap/THashMap.hpp"
#include "../CFunction/CFunction.hpp"

#pragma pack(push, 1)
struct CRTTISystem
{
	TDynArray<void *> builders;
	uint8_t pad0[4];
	THashMap<uint32_t, IRTTIType *> types;
	THashMap<uint32_t, CFunction *> functions;
};
#pragma pack(pop)

static_assert(offsetof(CRTTISystem, types) == 0x10);
static_assert(offsetof(CRTTISystem, functions) == 0x40);

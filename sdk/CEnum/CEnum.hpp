#pragma once
#include <cstdint>
#include <cstddef>
#include "../IRTTIType/IRTTIType.hpp"
#include "../TDynArray/TDynArray.hpp"
#include "../THashMap/THashMap.hpp"

#pragma pack(push, 1)
struct CEnum : IRTTIType
{
	CName name;
	TDynArray<CName> options;
	THashMap<uint32_t, int32_t> values;
	uint8_t pad0[0x34];
	uint8_t scripted;

	// value of option name, keyed by the CName index
	const int32_t *value(CName n) const
	{
		return values.find(n.index, n.index);
	}
};
#pragma pack(pop)

static_assert(offsetof(CEnum, options) == 0x0C);
static_assert(offsetof(CEnum, values) == 0x18);
static_assert(offsetof(CEnum, scripted) == 0x7C);

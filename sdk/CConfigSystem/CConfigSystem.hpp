#pragma once
#include <cstdint>
#include <cstddef>
#include "../CConfigVarRegistry/CConfigVarRegistry.hpp"

// Config::CConfigSystem singleton: only the registry pointer is needed.
#pragma pack(push, 1)
struct CConfigSystem
{
	uint8_t pad0[0x30];
	CConfigVarRegistry *registry;
};
#pragma pack(pop)

static_assert(offsetof(CConfigSystem, registry) == 0x30);

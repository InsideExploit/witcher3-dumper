#pragma once
#include <cstdint>
#include <cstddef>
#include "../IRTTIType/IRTTIType.hpp"
#include "../TDynArray/TDynArray.hpp"
#include "../CProperty/CProperty.hpp"
#include "../CFunction/CFunction.hpp"

enum EClassFlags : uint32_t
{
	CF_Abstract = 0x1,
	CF_Native = 0x2,
	CF_Scripted = 0x4,
	CF_Exported = 0x8,
	CF_State = 0x10,
	CF_NoDefaultDiff = 0x20,
	CF_Incomplete = 0x200,
	CF_StateMachine = 0x400,
};

#pragma pack(push, 1)
struct CClass : IRTTIType
{
	uint8_t pad0[0x08];
	CClass *base;
	uint8_t pad1[0x18];
	TDynArray<CProperty *> props;
	uint8_t pad2[0x0C];
	TDynArray<CFunction *> funcs;
	uint8_t pad3[0x10];
	uint32_t script_size;
	uint32_t flags;
};
#pragma pack(pop)

static_assert(offsetof(CClass, base) == 0x10);
static_assert(offsetof(CClass, props) == 0x30);
static_assert(offsetof(CClass, funcs) == 0x48);
static_assert(offsetof(CClass, script_size) == 0x64);
static_assert(offsetof(CClass, flags) == 0x68);

#pragma once
#include <cstdint>
#include <cstddef>
#include "../IRTTIType/IRTTIType.hpp"

enum EPropertyFlags : uint32_t
{
	PF_Editable = 0x1,
	PF_ReadOnly = 0x2,
	PF_Inlined = 0x4,
	PF_NotSerialized = 0x8,
	PF_NotCooked = 0x10,
	PF_Scripted = 0x20,
	PF_FuncParam = 0x80,
	PF_FuncLocal = 0x100,
	PF_Out = 0x200,
	PF_Optional = 0x400,
	PF_ImportVar = 0x2000,
	PF_Native = 0x4000,
	PF_Saved = 0x8000,
};

#pragma pack(push, 1)
struct CProperty
{
	void **vt;
	IRTTIType *type;
	CName name;
	uint8_t pad0[0x0C];
	uint32_t offset;
	uint32_t flags;
};
#pragma pack(pop)

static_assert(offsetof(CProperty, name) == 0x10);
static_assert(offsetof(CProperty, offset) == 0x20);
static_assert(offsetof(CProperty, flags) == 0x24);

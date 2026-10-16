#pragma once
#include <cstdint>
#include <cstddef>
#include "../CName/CName.hpp"
#include "../TDynArray/TDynArray.hpp"
#include "../CProperty/CProperty.hpp"

struct CClass;

enum EFunctionFlags : uint32_t
{
	FF_Native = 0x1,
	FF_Static = 0x2,
	FF_Exported = 0x8,
	FF_Final = 0x10,
	FF_Event = 0x20,
	FF_Latent = 0x40,
	FF_Entry = 0x80,
	FF_Exec = 0x100,
	FF_Undefined = 0x200,
	FF_Timer = 0x400,
	FF_StoryScene = 0x800,
	FF_Quest = 0x1000,
	FF_Cleanup = 0x2000,
	FF_Private = 0x4000,
	FF_Protected = 0x8000,
	FF_Public = 0x10000,
	FF_Reward = 0x20000,
};

#pragma pack(push, 1)
struct CFunction
{
	void **vt;
	CClass *owner;
	CName name;
	uint8_t pad0[4];
	uint32_t flags;
	uint8_t pad1[4];
	CProperty *ret;
	TDynArray<CProperty *> params;
	TDynArray<CProperty *> locals;
	uint8_t pad2[0x48];
	uint8_t *code;
	uint32_t code_size;
	uint8_t pad3[0x4C];
	uint32_t native_index;
};
#pragma pack(pop)

static_assert(offsetof(CFunction, flags) == 0x18);
static_assert(offsetof(CFunction, ret) == 0x20);
static_assert(offsetof(CFunction, params) == 0x28);
static_assert(offsetof(CFunction, locals) == 0x34);
static_assert(offsetof(CFunction, code) == 0x88);
static_assert(offsetof(CFunction, code_size) == 0x90);
static_assert(offsetof(CFunction, native_index) == 0xE0);

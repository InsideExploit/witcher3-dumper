#pragma once
#include <cstdint>
#include <cstddef>

enum EConfigVarType : uint8_t
{
	CVT_Bool = 0,
	CVT_Int = 1,
	CVT_Float = 2,
	CVT_String = 3,
};

// Config::IConfigVar / TConfigVar<T, Validation>. The value is read through the binding pointer,
// which points at the own value (+0x30, or +0x38 for strings) unless the cvar is bound elsewhere.
#pragma pack(push, 1)
struct IConfigVar
{
	void **vt;
	const char *group;
	const char *key;
	EConfigVarType type;
	uint8_t flags;
	uint8_t pad0[6];
	void *value;
	uint8_t def[0x10];
};
#pragma pack(pop)

static_assert(offsetof(IConfigVar, type) == 0x18);
static_assert(offsetof(IConfigVar, value) == 0x20);
static_assert(offsetof(IConfigVar, def) == 0x28);

#pragma once
#include <cstdint>
#include <cstddef>
#include "../IRTTIType/IRTTIType.hpp"

#pragma pack(push, 1)
struct CBitField : IRTTIType
{
	CName name;
	CName bits[32];
	uint32_t mask;
	uint32_t size;
};
#pragma pack(pop)

static_assert(offsetof(CBitField, bits) == 0x0C);
static_assert(offsetof(CBitField, mask) == 0x8C);
static_assert(offsetof(CBitField, size) == 0x90);

#pragma once
#include <cstdint>

struct CName
{
	uint32_t index;

	// CNamesPool text through the game's own lookup (takes the pool lock). "None" for index 0.
	const char *str() const;
};

static_assert(sizeof(CName) == 4);

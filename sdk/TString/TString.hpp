#pragma once
#include <cstdint>

// Next-gen TString<char>: size counts the terminating NUL, size <= 1 means empty.
#pragma pack(push, 4)
struct TString
{
	char *data;
	uint32_t size;

	const char *c_str() const
	{
		if (size <= 1 || !data)
			return "";

		return data;
	}
};
#pragma pack(pop)

#pragma once
#include <cstdint>

// Next-gen TDynArray is 12 bytes: the capacity is not stored, the allocator knows the block size.
#pragma pack(push, 4)
template <class T>
struct TDynArray
{
	T *data;
	uint32_t size;

	T *begin() const
	{
		return data;
	}

	T *end() const
	{
		return data ? data + size : data;
	}
};
#pragma pack(pop)

static_assert(sizeof(TDynArray<void *>) == 12);

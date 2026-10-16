#pragma once
#include <cstdint>
#include "../CName/CName.hpp"

enum ERTTITypeType : uint32_t
{
	RT_Simple = 0,
	RT_Enum = 1,
	RT_Class = 2,
	RT_Array = 3,
	RT_StaticArray = 4,
	RT_NativeArray = 5,
	RT_Pointer = 6,
	RT_Handle = 7,
	RT_SoftHandle = 8,
	RT_BitField = 9,
	RT_Fundamental = 11,
};

// Every reflected type (class, enum, bitfield, simple, wrapper) starts with this vtable.
struct IRTTIType
{
	void **vt;

	const CName *name() const
	{
		return ((const CName *(__fastcall *)(const IRTTIType *))vt[1])(this);
	}

	uint32_t size() const
	{
		return ((uint32_t(__fastcall *)(const IRTTIType *))vt[2])(this);
	}

	uint32_t alignment() const
	{
		return ((uint32_t(__fastcall *)(const IRTTIType *))vt[3])(this);
	}

	ERTTITypeType kind() const
	{
		return ((ERTTITypeType(__fastcall *)(const IRTTIType *))vt[4])(this);
	}

	const char *name_str() const
	{
		auto n = name();
		return n ? n->str() : "?";
	}
};

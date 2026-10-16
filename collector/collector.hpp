#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <utility>
#include "../sdk/IRTTIType/IRTTIType.hpp"
#include "../sdk/IConfigVar/IConfigVar.hpp"

struct CProperty;
struct CFunction;
struct IConfigVar;

struct prop_info
{
	std::string name;
	std::string type;
	ERTTITypeType kind = RT_Simple;
	uint32_t offset = 0;
	uint32_t flags = 0;
	uint32_t size = 0;
};

struct func_info
{
	std::string name;
	std::string owner;
	std::string ret;
	uint32_t flags = 0;
	uint32_t index = 0;
	uintptr_t rva = 0;
	uint32_t code_size = 0;
	std::vector<prop_info> params;
};

struct class_info
{
	std::string name;
	std::string parent;
	uint32_t size = 0;
	uint32_t align = 0;
	uint32_t flags = 0;
	uint32_t script_size = 0;
	std::vector<prop_info> props;
	std::vector<func_info> funcs;
};

struct enum_info
{
	std::string name;
	uint32_t size = 0;
	bool scripted = false;
	std::vector<std::pair<std::string, int32_t>> values;
};

struct bitfield_info
{
	std::string name;
	uint32_t size = 0;
	std::vector<std::pair<std::string, uint32_t>> bits;
};

struct simple_info
{
	std::string name;
	ERTTITypeType kind = RT_Simple;
	uint32_t size = 0;
	uint32_t align = 0;
};

struct cvar_info
{
	std::string group;
	std::string key;
	std::string value;
	std::string def;
	std::string validation;
	EConfigVarType type = CVT_Bool;
	uint8_t flags = 0;
	uintptr_t rva = 0;       // the cvar object
	uintptr_t value_rva = 0; // the value it reads and writes, 0 when bound to memory outside the module
};

// Reads the live reflection database, the global function map and the cvar registry.
class collector
{
public:
	static collector& get();

	bool counts(uint32_t& types, uint32_t& funcs) const;
	void collect();
	size_t exec_count() const;

	std::vector<class_info> classes;
	std::vector<enum_info> enums;
	std::vector<bitfield_info> bitfields;
	std::vector<simple_info> types;
	std::vector<func_info> globals;
	std::vector<cvar_info> cvars;

private:
	collector() = default;
	collector(const collector&) = delete;
	collector& operator=(const collector&) = delete;

	prop_info prop(const CProperty *p) const;
	func_info func(const CFunction *f) const;
	void collect_cvars();
};

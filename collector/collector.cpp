#include "collector.hpp"
#include <windows.h>
#include <dbghelp.h>
#include <algorithm>
#include <cstdio>
#include "../anchors/anchors.hpp"
#include "../utils/memory.hpp"
#include "../utils/logger.hpp"
#include "../sdk/CRTTISystem/CRTTISystem.hpp"
#include "../sdk/CClass/CClass.hpp"
#include "../sdk/CEnum/CEnum.hpp"
#include "../sdk/CBitField/CBitField.hpp"
#include "../sdk/CConfigSystem/CConfigSystem.hpp"
#include "../sdk/TString/TString.hpp"

collector& collector::get()
{
	static collector c;
	return c;
}

bool collector::counts(uint32_t& types_n, uint32_t& funcs_n) const
{
	CRTTISystem *rtti = anchors::get().rtti();
	if (!rtti)
		return false;

	types_n = rtti->types.count;
	funcs_n = rtti->functions.count;
	return true;
}

prop_info collector::prop(const CProperty *p) const
{
	prop_info r;
	if (!p)
		return r;

	r.name = p->name.str();
	r.offset = p->offset;
	r.flags = p->flags;

	if (p->type)
	{
		r.type = p->type->name_str();
		r.kind = p->type->kind();
		r.size = p->type->size();
	}

	return r;
}

func_info collector::func(const CFunction *f) const
{
	func_info r;
	r.name = f->name.str();
	r.flags = f->flags;
	r.code_size = f->code ? f->code_size : 0;
	r.ret = f->ret && f->ret->type ? f->ret->type->name_str() : "void";

	if (f->owner)
	{
		r.owner = ((const IRTTIType *)f->owner)->name_str();
	}

	for (CProperty *p : f->params)
	{
		r.params.push_back(prop(p));
	}

	if (f->flags & FF_Native)
	{
		r.index = f->native_index;
		r.rva = memory::get().rva(anchors::get().native(f->native_index, f->owner == nullptr));
	}

	return r;
}

// Validation template of a TConfigVar from its MSVC RTTI type name, e.g. "IntRange<0,63>".
static std::string validation(const IConfigVar *v)
{
	auto& mem = memory::get();
	if (!mem.in_module(v->vt))
		return "";

	auto col = (const uint32_t *)v->vt[-1];
	if (!mem.in_module(col) || col[0] != 1)
		return "";

	auto td = mem.base + col[3];
	if (!mem.in_module(td))
		return "";

	const char *raw = (const char *)td + 0x10;
	if (raw[0] != '.' || raw[1] != '?')
		return "";

	char out[512] = {};
	// 0x2800 = UNDNAME_32_BIT_DECODE | UNDNAME_TYPE_ONLY, the flags type_info::name uses for ".?AV" names
	if (!UnDecorateSymbolName(raw + 1, out, sizeof(out), 0x2800))
		return raw;

	std::string s = out;
	size_t at = s.find("Validation::");
	if (at == std::string::npos)
		return s;

	s = s.substr(at + 12);
	while (!s.empty() && (s.back() == '>' || s.back() == ' '))
	{
		s.pop_back();
	}

	// the outer TConfigVar closer was stripped together with the inner one, put the inner back
	if (s.find('<') != std::string::npos)
	{
		s.push_back('>');
	}

	return s;
}

static std::string cvar_text(EConfigVarType type, const void *p)
{
	if (!p)
		return "";

	char buf[64];
	switch (type)
	{
	case CVT_Bool:
		return *(const uint8_t *)p ? "true" : "false";
	case CVT_Int:
		snprintf(buf, sizeof(buf), "%d", *(const int32_t *)p);
		return buf;
	case CVT_Float:
		snprintf(buf, sizeof(buf), "%g", *(const float *)p);
		return buf;
	case CVT_String:
		return ((const TString *)p)->c_str();
	}

	return "";
}

void collector::collect_cvars()
{
	CConfigSystem *cfg = anchors::get().config();
	if (!cfg || !cfg->registry)
	{
		logger::get().info("cvars: config system not available");
		return;
	}

	CConfigVarRegistry *reg = cfg->registry;
	EnterCriticalSection(&reg->lock);

	reg->vars.each([&](uint32_t, IConfigVar *v)
	{
		if (!v || !v->group || !v->key)
			return;

		cvar_info c;
		c.group = v->group;
		c.key = v->key;
		c.type = v->type;
		c.flags = v->flags;
		c.value = cvar_text(v->type, v->value);
		c.def = cvar_text(v->type, v->def);
		c.validation = validation(v);
		c.rva = memory::get().rva(v);
		c.value_rva = memory::get().rva(v->value);
		cvars.push_back(c);
	});

	LeaveCriticalSection(&reg->lock);

	std::sort(cvars.begin(), cvars.end(), [](const cvar_info& a, const cvar_info& b)
	{
		if (a.group != b.group)
			return a.group < b.group;

		return a.key < b.key;
	});
}

void collector::collect()
{
	classes.clear();
	enums.clear();
	bitfields.clear();
	types.clear();
	globals.clear();
	cvars.clear();

	CRTTISystem *rtti = anchors::get().rtti();
	if (!rtti)
		return;

	rtti->types.each([&](uint32_t, IRTTIType *t)
	{
		if (!t)
			return;

		ERTTITypeType kind = t->kind();
		if (kind == RT_Class)
		{
			auto c = (const CClass *)t;
			class_info ci;
			ci.name = t->name_str();
			ci.parent = c->base ? c->base->name_str() : "";
			ci.size = t->size();
			ci.align = t->alignment();
			ci.flags = c->flags;
			ci.script_size = c->script_size;

			for (CProperty *p : c->props)
			{
				ci.props.push_back(prop(p));
			}

			for (CFunction *f : c->funcs)
			{
				if (f)
				{
					ci.funcs.push_back(func(f));
				}
			}

			std::sort(ci.props.begin(), ci.props.end(), [](const prop_info& a, const prop_info& b)
			{
				return a.offset < b.offset;
			});

			classes.push_back(ci);
			return;
		}

		if (kind == RT_Enum)
		{
			auto e = (const CEnum *)t;
			enum_info ei;
			ei.name = t->name_str();
			ei.size = t->size();
			ei.scripted = e->scripted != 0;

			for (const CName& o : e->options)
			{
				int32_t value = 0;
				const int32_t *v = e->value(o);
				if (v)
				{
					value = *v;
				}
				else
				{
					e->values.each([&](uint32_t k, int32_t val)
					{
						if (k == o.index)
						{
							value = val;
						}
					});
				}

				ei.values.push_back({ o.str(), value });
			}

			enums.push_back(ei);
			return;
		}

		if (kind == RT_BitField)
		{
			auto b = (const CBitField *)t;
			bitfield_info bi;
			bi.name = t->name_str();
			bi.size = b->size;

			for (uint32_t i = 0; i < 32; ++i)
			{
				if (b->mask & (1u << i))
				{
					bi.bits.push_back({ b->bits[i].str(), i });
				}
			}

			bitfields.push_back(bi);
			return;
		}

		simple_info si;
		si.name = t->name_str();
		si.kind = kind;
		si.size = t->size();
		si.align = t->alignment();
		types.push_back(si);
	});

	rtti->functions.each([&](uint32_t, CFunction *f)
	{
		if (f)
		{
			globals.push_back(func(f));
		}
	});

	collect_cvars();

	auto by_name = [](const auto& a, const auto& b)
	{
		return a.name < b.name;
	};

	std::sort(classes.begin(), classes.end(), by_name);
	std::sort(enums.begin(), enums.end(), by_name);
	std::sort(bitfields.begin(), bitfields.end(), by_name);
	std::sort(types.begin(), types.end(), by_name);
	std::sort(globals.begin(), globals.end(), by_name);

	logger::get().info("collected %zu classes, %zu enums, %zu bitfields, %zu other types, %zu global functions (%zu exec), %zu cvars", classes.size(), enums.size(), bitfields.size(), types.size(), globals.size(), exec_count(), cvars.size());
}

size_t collector::exec_count() const
{
	size_t n = 0;
	for (const func_info& f : globals)
	{
		if (f.flags & FF_Exec)
		{
			++n;
		}
	}

	return n;
}

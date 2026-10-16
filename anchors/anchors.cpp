#include "anchors.hpp"
#include "../utils/memory.hpp"
#include "../utils/logger.hpp"
#include "../sdk/CRTTISystem/CRTTISystem.hpp"
#include "../sdk/CConfigSystem/CConfigSystem.hpp"

// SRTTI::GetInstance: mov rax, cs:<CRTTISystem*> at +4
static const char *pat_rtti = "48 83 EC 28 48 8B 05 ? ? ? ? 48 85 C0 0F 85 ? ? ? ? E8 ? ? ? ? 48 8D 48 ? FF 15 ? ? ? ? 48 8B 05 ? ? ? ? 48 85 C0 0F 85 ? ? ? ? 38 05 ? ? ? ? 74 ? 88 05 ? ? ? ? 33 C0 48 8D 0D ? ? ? ? 48 89 05";

// CName::AsChar: CNamesPool lock, holders at pool+0x11830, holder->text
static const char *pat_as_char = "48 89 5C 24 ? 57 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 63 1B 48 8B C8 48 8B F8 FF 15 ? ? ? ? 48 8B 8F ? ? ? ? 48 8B 1C D9 48 8B CF";

// Config::CConfigSystem getter: mov rax, cs:<CConfigSystem*> at +4
static const char *pat_config = "48 83 EC 28 48 8B 05 ? ? ? ? 48 85 C0 75 ? E8 ? ? ? ? 48 8D 48 ? FF 15 ? ? ? ? 48 8B 05 ? ? ? ? 48 85 C0 75 ? 38 05 ? ? ? ? 74 ? 88 05 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 89 05 ? ? ? ? 48 8B 05";

// CFunction::CFunction(CName, globalFn): lea rdx, m_nativeGlobals
static const char *pat_global_ctor = "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 33 F6 48 8D 05 ? ? ? ? 48 89 01 48 8B F9";

// CFunction::CFunction(CClass*, CName, memberFn): lea r8, m_nativeFunctions
static const char *pat_class_ctor = "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 33 F6 48 89 51 ? 48 8D 05 ? ? ? ? 48 8B F9";

anchors& anchors::get()
{
	static anchors a;
	return a;
}

static uint8_t *find_one(const char *what, const char *pattern)
{
	int n = 0;
	uint8_t *p = memory::get().find_text(pattern, &n);
	if (!p)
	{
		logger::get().info("anchor %s: not found", what);
		return nullptr;
	}

	logger::get().info("anchor %s: rva 0x%llX (%d match%s)", what, (unsigned long long)memory::get().rva(p), n, n == 1 ? "" : "es, using the first");
	return p;
}

bool anchors::resolve()
{
	auto& mem = memory::get();

	uint8_t *p = find_one("rtti", pat_rtti);
	if (p)
	{
		rtti_ptr = (void **)memory::rip(p + 4, 3, 7);
	}

	p = find_one("as_char", pat_as_char);
	if (p)
	{
		as_char = (const char *(__fastcall *)(const uint32_t *))p;
	}

	p = find_one("config", pat_config);
	if (p)
	{
		config_ptr = (void **)memory::rip(p + 4, 3, 7);
	}

	p = find_one("global_ctor", pat_global_ctor);
	if (p)
	{
		uint8_t *lea = mem.find("48 8D 15 ? ? ? ?", p, 0x90);
		if (lea)
		{
			global_natives = (void **)memory::rip(lea, 3, 7);
		}

		if (!mem.find("89 87 E0 00 00 00", p, 0xA0))
		{
			logger::get().info("warning: native index store at +0xE0 not seen in the global ctor");
		}
	}

	p = find_one("class_ctor", pat_class_ctor);
	if (p)
	{
		uint8_t *lea = mem.find("4C 8D 05 ? ? ? ?", p, 0x90);
		if (lea)
		{
			class_natives = memory::rip(lea, 3, 7);
		}
	}

	// build banner, e.g. "5.0.1048522 P4CL: 13413172 Stream: //Red_engine/Main.Lava.Release"
	uint8_t *v = mem.find("50 34 43 4C 3A 20", mem.rdata, mem.rdata_size);
	if (v)
	{
		uint8_t *s = v;
		while (s > mem.rdata)
		{
			uint8_t c = s[-1];
			if (c < 0x20 && c != '\n' && c != '\r')
				break;

			--s;
		}

		for (uint8_t *c = s; *c && c - s < 160; ++c)
		{
			version.push_back(*c == '\n' || *c == '\r' ? ' ' : (char)*c);
		}
	}

	// ImageBase from the file on disk; the loader may have rewritten the in-memory header
	wchar_t path[MAX_PATH];
	GetModuleFileNameW(nullptr, path, MAX_PATH);

	FILE *f = nullptr;
	_wfopen_s(&f, path, L"rb");
	if (f)
	{
		uint8_t hdr[0x1000] = {};
		fread(hdr, 1, sizeof(hdr), f);
		fclose(f);

		auto dos = (IMAGE_DOS_HEADER *)hdr;
		if (dos->e_magic == IMAGE_DOS_SIGNATURE && dos->e_lfanew > 0 && dos->e_lfanew < 0xE00)
		{
			preferred_base = ((IMAGE_NT_HEADERS64 *)(hdr + dos->e_lfanew))->OptionalHeader.ImageBase;
		}
	}

	logger::get().info("module base 0x%llX, preferred base 0x%llX", (unsigned long long)mem.base, (unsigned long long)preferred_base);
	logger::get().info("rtti ptr rva 0x%llX, config ptr rva 0x%llX, natives rva 0x%llX / 0x%llX", (unsigned long long)mem.rva(rtti_ptr), (unsigned long long)mem.rva(config_ptr), (unsigned long long)mem.rva(global_natives), (unsigned long long)mem.rva(class_natives));
	logger::get().info("version: %s", version.empty() ? "unknown" : version.c_str());

	if (!rtti_ptr || !as_char)
		return false;

	return true;
}

CRTTISystem *anchors::rtti() const
{
	return rtti_ptr ? (CRTTISystem *)*rtti_ptr : nullptr;
}

CConfigSystem *anchors::config() const
{
	return config_ptr ? (CConfigSystem *)*config_ptr : nullptr;
}

void *anchors::native(uint32_t index, bool global) const
{
	// slots below 256 are opcode handlers; an import without a C++ function keeps index 0
	if (index < 256 || index >= 4096)
		return nullptr;

	if (global)
		return global_natives ? global_natives[index] : nullptr;

	if (!class_natives)
		return nullptr;

	return *(void **)(class_natives + index * 24);
}

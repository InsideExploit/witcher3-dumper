#pragma once
#include <windows.h>
#include <cstdint>
#include <cstring>
#include <vector>

class memory
{
public:
	static memory& get()
	{
		static memory m;
		return m;
	}

	bool init(HMODULE module)
	{
		base = (uint8_t *)module;
		if (!base)
			return false;

		auto dos = (IMAGE_DOS_HEADER *)base;
		auto nt = (IMAGE_NT_HEADERS64 *)(base + dos->e_lfanew);
		size = nt->OptionalHeader.SizeOfImage;

		auto sec = IMAGE_FIRST_SECTION(nt);
		for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++sec)
		{
			if (!memcmp(sec->Name, ".text", 6))
			{
				text = base + sec->VirtualAddress;
				text_size = sec->Misc.VirtualSize;
			}

			if (!memcmp(sec->Name, ".rdata", 7))
			{
				rdata = base + sec->VirtualAddress;
				rdata_size = sec->Misc.VirtualSize;
			}
		}

		return text && rdata;
	}

	// IDA style pattern, "?" is a wildcard byte. Returns the first match and the number of matches.
	uint8_t *find(const char *pattern, uint8_t *start, size_t len, int *count = nullptr)
	{
		std::vector<int> pat;
		for (const char *p = pattern; *p;)
		{
			if (*p == ' ')
			{
				++p;
				continue;
			}

			if (*p == '?')
			{
				pat.push_back(-1);
				while (*p == '?')
				{
					++p;
				}

				continue;
			}

			pat.push_back((int)strtoul(p, nullptr, 16));
			p += 2;
		}

		uint8_t *first = nullptr;
		int n = 0;
		size_t plen = pat.size();
		if (!plen || len < plen)
			return nullptr;

		for (size_t i = 0; i <= len - plen; ++i)
		{
			size_t j = 0;
			while (j < plen && (pat[j] < 0 || start[i + j] == (uint8_t)pat[j]))
			{
				++j;
			}

			if (j != plen)
				continue;

			if (!first)
			{
				first = start + i;
			}

			++n;
		}

		if (count)
		{
			*count = n;
		}

		return first;
	}

	uint8_t *find_text(const char *pattern, int *count = nullptr)
	{
		return find(pattern, text, text_size, count);
	}

	// Target of a rip-relative operand: disp32 at insn+disp_off, instruction length len.
	static uint8_t *rip(uint8_t *insn, int disp_off, int len)
	{
		return insn + len + *(int32_t *)(insn + disp_off);
	}

	uintptr_t rva(const void *p) const
	{
		if (!p)
			return 0;

		auto a = (uintptr_t)p;
		auto b = (uintptr_t)base;
		if (a < b || a >= b + size)
			return 0;

		return a - b;
	}

	bool in_module(const void *p) const
	{
		return rva(p) != 0;
	}

	uint8_t *base = nullptr;
	size_t size = 0;
	uint8_t *text = nullptr;
	size_t text_size = 0;
	uint8_t *rdata = nullptr;
	size_t rdata_size = 0;

private:
	memory() = default;
	memory(const memory&) = delete;
	memory& operator=(const memory&) = delete;
};

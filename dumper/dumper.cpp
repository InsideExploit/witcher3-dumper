#include "dumper.hpp"
#include <windows.h>
#include <shlobj.h>
#include "../anchors/anchors.hpp"
#include "../collector/collector.hpp"
#include "../writer/writer.hpp"
#include "../utils/memory.hpp"
#include "../utils/logger.hpp"

dumper& dumper::get()
{
	static dumper d;
	return d;
}

// Documents\witcher3-dumper, created on demand. The game folder usually needs elevation.
std::wstring dumper::out_dir() const
{
	std::wstring dir = L".";
	PWSTR docs = nullptr;
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &docs)))
	{
		dir = docs;
	}

	CoTaskMemFree(docs);
	dir += L"\\witcher3-dumper";
	CreateDirectoryW(dir.c_str(), nullptr);
	return dir;
}

void dumper::run()
{
	std::wstring dir = out_dir();
	logger::get().open(dir + L"\\log.txt");
	logger::get().info("witcher3-dumper attached");

	if (!memory::get().init(GetModuleHandleW(nullptr)))
	{
		logger::get().info("cannot parse the game module");
		return;
	}

	if (!anchors::get().resolve())
	{
		logger::get().info("required anchors missing, nothing dumped (is this the next-gen 5.x witcher3.exe?)");
		return;
	}

	// Scripted classes, script functions and exec commands only exist after the scripts are loaded.
	// Dump once the type and function counts have been stable for 5 s and exec functions exist.
	// F8 dumps immediately, and after 15 minutes the dump happens regardless.
	uint32_t last_types = 0;
	uint32_t last_funcs = 0;
	int stable = 0;
	bool force = false;
	DWORD start = GetTickCount();

	for (;;)
	{
		Sleep(500);

		if (GetAsyncKeyState(VK_F8) & 1)
		{
			logger::get().info("F8 pressed, dumping now");
			force = true;
		}

		if (GetTickCount() - start > 15 * 60 * 1000)
		{
			logger::get().info("timeout, dumping what is loaded");
			force = true;
		}

		uint32_t types = 0;
		uint32_t funcs = 0;
		if (!collector::get().counts(types, funcs))
		{
			if (force)
			{
				logger::get().info("rtti system not created yet, giving up");
				return;
			}

			continue;
		}

		if (types != last_types || funcs != last_funcs)
		{
			logger::get().info("waiting: %u types, %u global functions", types, funcs);
			last_types = types;
			last_funcs = funcs;
			stable = 0;
		}
		else
		{
			++stable;
		}

		if (!force && (stable < 10 || types < 1000))
			continue;

		collector::get().collect();
		if (!force && !collector::get().exec_count())
		{
			// retry after another 15 s of stability
			logger::get().info("no exec functions yet, scripts not loaded; waiting");
			stable = -20;
			continue;
		}

		break;
	}

	std::string tag = anchors::get().version.substr(0, anchors::get().version.find(' '));
	std::wstring sub = dir + L"\\" + (tag.empty() ? std::wstring(L"dump") : std::wstring(tag.begin(), tag.end()));
	writer::get().write(sub);
	logger::get().info("done");
}

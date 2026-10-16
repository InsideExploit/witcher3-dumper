#include <windows.h>
#include "dumper/dumper.hpp"
#include "utils/logger.hpp"

static HMODULE self = nullptr;
static HANDLE thread = nullptr;

static DWORD WINAPI worker(LPVOID)
{
	dumper::get().run();
	logger::get().close();
	FreeLibraryAndExitThread(self, 0);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		self = module;
		DisableThreadLibraryCalls(module);
		thread = CreateThread(nullptr, 0, worker, nullptr, 0, nullptr);
		return TRUE;
	}

	if (reason == DLL_PROCESS_DETACH)
	{
		logger::get().close();
		if (thread)
		{
			CloseHandle(thread);
			thread = nullptr;
		}
	}

	return TRUE;
}

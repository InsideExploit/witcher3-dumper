#pragma once
#include <windows.h>
#include <cstdio>
#include <cstdarg>
#include <string>

class logger
{
public:
	static logger& get()
	{
		static logger l;
		return l;
	}

	void open(const std::wstring& path)
	{
		if (file)
			return;

		_wfopen_s(&file, path.c_str(), L"w");
	}

	void close()
	{
		if (!file)
			return;

		fclose(file);
		file = nullptr;
	}

	void info(const char *fmt, ...)
	{
		char buf[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buf, sizeof(buf), fmt, args);
		va_end(args);

		OutputDebugStringA("[witcher3-dumper] ");
		OutputDebugStringA(buf);
		OutputDebugStringA("\n");

		if (!file)
			return;

		fputs(buf, file);
		fputs("\n", file);
		fflush(file);
	}

private:
	logger() = default;
	logger(const logger&) = delete;
	logger& operator=(const logger&) = delete;

	FILE *file = nullptr;
};

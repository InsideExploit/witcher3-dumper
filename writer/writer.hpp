#pragma once
#include <string>

// Writes the collected model: Classes.hpp, Enums.hpp, Asserts.hpp and the text listings.
class writer
{
public:
	static writer& get();

	bool write(const std::wstring& dir);

private:
	writer() = default;
	writer(const writer&) = delete;
	writer& operator=(const writer&) = delete;

	void classes_hpp(const std::wstring& dir);
	void enums_hpp(const std::wstring& path);
	void cvars_ini(const std::wstring& path);
	void exec_html(const std::wstring& path);
};

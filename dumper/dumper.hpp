#pragma once
#include <string>

// Waits until the reflection database and the compiled scripts are loaded, then dumps everything once.
class dumper
{
public:
	static dumper& get();

	void run();

private:
	dumper() = default;
	dumper(const dumper&) = delete;
	dumper& operator=(const dumper&) = delete;

	std::wstring out_dir() const;
};

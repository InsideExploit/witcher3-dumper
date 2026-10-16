#pragma once
#include <cstdint>
#include <string>

struct CRTTISystem;
struct CConfigSystem;

// Engine entry points found by byte pattern in the running witcher3.exe.
class anchors
{
public:
	static anchors& get();

	bool resolve();

	CRTTISystem *rtti() const;
	CConfigSystem *config() const;

	// address of a class native (24-byte member pointer slot) or a global native
	void *native(uint32_t index, bool global) const;

	const char *(__fastcall *as_char)(const uint32_t *name) = nullptr;
	void **rtti_ptr = nullptr;
	void **config_ptr = nullptr;
	void **global_natives = nullptr;
	uint8_t *class_natives = nullptr;
	uint64_t preferred_base = 0;
	std::string version;

private:
	anchors() = default;
	anchors(const anchors&) = delete;
	anchors& operator=(const anchors&) = delete;
};

#include "CName.hpp"
#include "../../anchors/anchors.hpp"

const char *CName::str() const
{
	auto fn = anchors::get().as_char;
	if (!fn)
		return "?";

	return fn(&index);
}

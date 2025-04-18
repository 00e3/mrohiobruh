#pragma once
#include <functional>

namespace GUI::Controls {
	void Button( std::string name, std::function< void( ) > callback, bool use_unique_id = false );
}

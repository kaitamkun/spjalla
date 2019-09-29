#ifndef SPJALLA_CONFIG_KEYS_H_
#define SPJALLA_CONFIG_KEYS_H_

#include "haunted/core/key.h"

namespace spjalla::config {
	struct keys {
		static haunted::key toggle_overlay, switch_server, next_window, previous_window;
	};
}

#endif

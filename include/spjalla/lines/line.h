#ifndef SPJALLA_LINES_LINES_H_
#define SPJALLA_LINES_LINES_H_

#include <string>

#include "pingpong/core/util.h"
#include "haunted/ui/textbox.h"
#include "spjalla/core/notifications.h"
#include "lib/formicine/ansi.h"

namespace spjalla {
	class client;
}

namespace spjalla::lines {
	extern std::string notice, red_notice, yellow_notice;
	extern size_t time_length; // The length of render_time's returned values.

	/** Renders a UNIX timestamp as an hours-minutes-seconds set. */
	std::string render_time(long stamp, bool with_ansi = true);

	struct line: haunted::ui::textline {
		client *parent;
		long stamp;

		line(client *parent_, long stamp_ = pingpong::util::timestamp(), int continuation_ = 0):
			haunted::ui::textline(continuation_ + time_length), parent(parent_), stamp(stamp_) {}

		virtual notification_type get_notification_type() const { return notification_type::none; }
	};

	/** Returns the current timestamp in seconds. */
	long now();
}

#endif

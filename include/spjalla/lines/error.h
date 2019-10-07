#ifndef SPJALLA_LINES_ERROR_H_
#define SPJALLA_LINES_ERROR_H_

#include "spjalla/lines/line.h"

namespace spjalla::lines {
	struct error_line: public line {
		std::string message;
		long stamp;

		error_line(const std::string &message_, long stamp_ = pingpong::util::timestamp()):
			line(0), message(message_), stamp(stamp_) {}

		virtual operator std::string() const override;
		virtual notification_type get_notification_type() const override { return notification_type::highlight; }
	};
}

#endif

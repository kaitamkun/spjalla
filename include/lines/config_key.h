#ifndef SPJALLA_LINES_CONFIG_KEY_H_
#define SPJALLA_LINES_CONFIG_KEY_H_

#include "pingpong/core/pputil.h"

#include "core/config.h"
#include "lines/lines.h"

namespace spjalla::lines {
	struct config_key_line: public haunted::ui::textline {
		std::string key;
		config_value value;
		long stamp;

		config_key_line(const std::string &key_, const config_value &value_, long stamp_ = pingpong::util::timestamp()):
			haunted::ui::textline(4 + key_.length() + 3), key(key_), value(value_), stamp(stamp_) {}

		config_key_line(const std::pair<std::string, config_value> &pair, long stamp_ = pingpong::util::timestamp()):
			config_key_line(pair.first, pair.second, stamp_) {}

		virtual operator std::string() const override;
	};
}

#endif
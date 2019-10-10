#include "pingpong/core/util.h"
#include "spjalla/plugins/logs/log_line.h"
#include "lib/formicine/ansi.h"

namespace spjalla::plugins::logs {
	log_line::operator std::string() const {
		return "───── "_d + "Log " + verb + " on " + ansi::bold(pingpong::util::get_date(stamp)) + " ─────"_d;
	}
}

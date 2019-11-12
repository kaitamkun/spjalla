#ifndef SPJALLA_LINES_NOTICE_H_
#define SPJALLA_LINES_NOTICE_H_

#include "pingpong/core/defs.h"
#include "pingpong/core/hats.h"
#include "pingpong/core/local.h"

#include "pingpong/commands/notice.h"
#include "pingpong/events/notice.h"

#include "spjalla/lines/message.h"

namespace spjalla::lines {
	class notice_line: public message_line {
		private:
			bool always_highlight = false;

		public:
			notice_line(client *, std::shared_ptr<pingpong::user>, const std::string &where_,
			             const std::string &message_, long stamp_, bool direct_only_ = false);

			notice_line(client *, const std::string &name_, const std::string &where_, const std::string &self_,
			             const std::string &message_, long, const pingpong::hat_set &, bool direct_only_ = false);

			notice_line(client *, const std::string &combined_, const std::string &where_, const std::string &self_,
			             const std::string &message_, long, bool direct_only_ = false);

			notice_line(client *parent_, const pingpong::privmsg_command &cmd, bool direct_only_ = false):
				notice_line(parent_, cmd.serv->get_self(), cmd.where, cmd.message, cmd.sent_time, direct_only_) {}

			notice_line(client *parent_, const pingpong::privmsg_event &ev, bool direct_only_ = false):
				notice_line(parent_, ev.speaker, ev.where, ev.content, ev.stamp, direct_only_) {}

			notification_type get_notification_type() const override;

			static std::string to_string(const pingpong::privmsg_event &);
	};
}

#endif

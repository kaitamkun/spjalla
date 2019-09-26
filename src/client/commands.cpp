#include "pingpong/core/debug.h"

#include "pingpong/commands/join.h"
#include "pingpong/commands/kick.h"
#include "pingpong/commands/mode.h"
#include "pingpong/commands/nick.h"
#include "pingpong/commands/part.h"
#include "pingpong/commands/privmsg.h"
#include "pingpong/commands/quit.h"

#include "pingpong/net/resolution_error.h"

#include "core/client.h"
#include "lines/lines.h"
#include "lines/warning.h"
#include "formicine/futil.h"

namespace spjalla {
	void client::add_commands() {
		using sptr = pingpong::server *;
		using line = const input_line &;

		pingpong::command::before_send = [&](pingpong::command &cmd) { return before_send(cmd); };

		add({"ban", {1, 2, true, [&](sptr serv, line il) {
			ban(serv, il, "+b");
		}}});

		add({"clear", {0, 0, false, [&](sptr, line) {
			if (ui::window *win = ui.get_active_window()) {
				win->set_voffset(win->total_rows());
			} else {
				DBG(lines::red_notice + "No window.");
			}
		}}});

		add({"connect", {1, 2, false, [&](sptr, line il) {
			const std::string &where = il.first();

			std::string nick(pingpong::irc::default_nick);
			if (il.args.size() > 1)
				nick = il.args[1];

			std::string hostname;
			long port = 0;

			std::tie(hostname, port) = pp.connect(where, nick, 6667, [&](const std::function<void()> &fn) {
				try {
					fn();
				} catch (const std::exception &err) {
					ui.log(lines::warning_line("Couldn't connect to " + ansi::bold(hostname) + " on port " +
						ansi::bold(std::to_string(port)) + ": " + err.what()));
				}
			});

			ui.log("Connecting to " + ansi::bold(hostname) + " on port " + ansi::bold(std::to_string(port)) + "...");
		}}});
		
		add({"dbg", {0, 0, false, [&](sptr, line) {
			debug_servers();
		}}});

		add({"defnick", {0, 1, false, [&](sptr, line il) {
			if (il.args.empty()) {
				ui.log("The current default nick is " + ansi::bold(pingpong::irc::default_nick));
			} else {
				pingpong::irc::default_nick = il.first();
				ui.log("Default nick set to " + ansi::bold(pingpong::irc::default_nick));
			}
		}}});

		add({"defuser", {0, 1, false, [&](sptr, line il) {
			if (il.args.empty()) {
				ui.log("The current default user is " + ansi::bold(pingpong::irc::default_user));
			} else {
				pingpong::irc::default_user = il.first();
				ui.log("Default username set to " + ansi::bold(pingpong::irc::default_user));
			}
		}}});

		add({"defreal", {0, -1, false, [&](sptr, line il) {
			if (il.args.empty()) {
				ui.log("The current default realname is " + ansi::bold(pingpong::irc::default_realname));
			} else {
				pingpong::irc::default_realname = il.body;
				ui.log("Default realname set to " + ansi::bold(pingpong::irc::default_realname));
			}
		}}});

		add({"info", {0, 1, false, [&](sptr, line il) {
			if (il.args.size() == 0) {
				pingpong::debug::print_all(pp);
				return;
			}
			
			const std::string &first = il.first();
			ui.log("Unknown option: " + first);
		}}});

		add<pingpong::join_command>("join");

		add({"kick", {1, -1, true, [&](sptr serv, line il) {
			if (triple_command<pingpong::kick_command>(serv, il, ui.get_active_channel()))
				no_channel();
		}}});

		add({"me", {1, -1, true, [&](sptr, line il) {
			const ui::window *win = ui.active_window;
			if (win->is_dead())
				return;

			const std::string msg = "\1ACTION " + il.body + "\1";
			if (win->is_channel())
				pingpong::privmsg_command(win->data.chan, msg).send();
			else if (win->is_user())
				pingpong::privmsg_command(win->data.user, msg).send();
		}}});

		add({"mode", {1, -1, true, [&](sptr serv, line il) {
			std::shared_ptr<pingpong::channel> win_chan = ui.get_active_channel();

			if (il.args.size() == 1 && il.first().find_first_of("+-") == 0) {
				// The only argument is a set of flags. If the active window is a channel, this sets the flags on the
				// channel. Otherwise, the flags are set on yourself.
				if (win_chan)
					pingpong::mode_command(win_chan, il.first()).send();
				else
					pingpong::mode_command(serv->get_nick(), serv, il.first()).send();
				return;
			}

			std::string chan_str {}, flags {}, extra {};

			// Look through all the arguments.
			for (const std::string &arg: il.args) {
				char front = arg.front();
				if (front == '#') {
					if (!chan_str.empty()) {
						ui.warn("You cannot set modes for multiple channels in one /mode command.");
						return;
					}

					chan_str = arg;
				} else if (front == '+' || front == '-') {
					if (arg.find_first_not_of(pingpong::util::flag_chars) != std::string::npos) {
						ui.warn("Invalid flags for mode command: " + arg);
						return;
					}

					if (flags.empty()) {
						flags = arg;
					} else {
						ui.warn("You cannot set multiple sets of flags in one /mode command.");
						return;
					}
				} else if (extra.empty()) {
					extra = arg;
				} else {
					// No overwriting the extra parameters.
					ui.warn("You cannot set flags for multiple targets in one /mode command.");
					return;
				}
			}

			// You can't set modes without flags.
			if (flags.empty()) {
				ui.warn("No flags specified for /mode.");
				return;
			}

			// If there's no channel indicated either in the command arguments or by the active window and you're not
			// trying to set user flags on yourself, then what are you even trying to do? You can't set flags on someone
			// who isn't you.
			if (!win_chan && chan_str.empty() && extra != serv->get_nick()) {
				ui.warn("Invalid arguments for /mode.");
				return;
			}

			// If there's no channel and we're setting arguments on ourself, it's a regular user mode command.
			if (!win_chan && chan_str.empty() && extra == serv->get_nick() && !flags.empty()) {
				pingpong::mode_command(serv->get_self(), flags).send();
				return;
			}

			if (chan_str.empty() && win_chan)
				chan_str = win_chan->name;

			// At this point, I think it's safe to assume that you're setting channel flags. The extra parameter, if
			// present, is what/whom you're setting the flags on.
			pingpong::mode_command(chan_str, serv, flags, extra).send();
		}}});

		add({"msg", {2, -1, true, [&](sptr serv, line il) {
			pingpong::privmsg_command(serv, il.first(), il.rest()).send();
		}}});

		add({"nick", {0,  1, true, [&](sptr serv, line il) {
			if (il.args.size() == 0)
				ui.log("Current nick: " + serv->get_nick());
			else
				pingpong::nick_command(serv, il.first()).send();
		}}});

		add({"overlay", {0, 0, false, [&](sptr, line) {
			ui.update_overlay();
		}}});

		add({"part", {0, -1, true, [&](sptr serv, line il) {
			std::shared_ptr<pingpong::channel> active_channel = ui.get_active_channel();

			if ((il.args.empty() || il.first()[0] != '#') && !active_channel) {
				no_channel();
			} else if (il.args.empty()) {
				pingpong::part_command(serv, active_channel).send();
			} else if (il.first()[0] != '#') {
				pingpong::part_command(serv, active_channel, il.body).send();
			} else if (std::shared_ptr<pingpong::channel> cptr = serv->get_channel(il.first())) {
				pingpong::part_command(serv, cptr, il.rest()).send();
			} else {
				ui.log(il.first() + ": no such channel.");
			}
		}}});

		add({"quit", {0, -1, false, [&](sptr, line il) {
			if (il.args.empty()) {
				for (auto serv: pp.server_order)
					pingpong::quit_command(serv).send();
			} else {
				for (auto serv: pp.server_order)
					pingpong::quit_command(serv, il.body).send();
			}

			stop();
		}}});

		add({"quote", {1, -1, true, [&](sptr serv, line il) {
			serv->quote(il.body);
		}}});

		add({"spam", {0, 1, false, [&](sptr, line il) {
			long max = 64;

			if (!il.args.empty()) {
				char *ptr;
				const std::string &str = il.first();
				long parsed = strtol(str.c_str(), &ptr, 10);
				if (ptr != 1 + &*(str.end() - 1)) {
					ui.log(ansi::yellow("!!") + " Invalid number: \"" + il.first() + "\"");
				} else max = parsed;
			}

			for (long i = 1; i <= max; ++i)
				ui.log(std::to_string(i));
		}}});

		add({"unban", {1, 2, true, [&](sptr serv, line il) {
			ban(serv, il, "-b");
		}}});

		add({"wc", {0, 0, false, [&](sptr, line) {
			if (ui.can_remove())
				ui.remove_window(ui.active_window);
		}}});
	}

	void client::ban(pingpong::server *serv, const input_line &il, const std::string &type) {
		std::shared_ptr<pingpong::channel> chan = ui.get_active_channel();
		std::string target;

		if (il.args.size() == 2) {
			if (il.args[0].front() != '#') {
				ui.warn("Invalid channel name: " + il.args[0]);
				return;
			}

			chan = serv->get_channel(il.args[0], false);
			if (!chan) {
				ui.warn("Channel not found: " + il.args[0]);
				return;
			}

			target = il.args[1];
		} else {
			target = il.args[0];
		}

		if (!chan) {
			ui.warn("Cannot ban: no channel specified.");
			return;
		}

		pingpong::mode_command(chan, type, target).send();
	}

	void client::debug_servers() {
		if (pp.servers.empty()) {
			DBG("No servers.");
			return;
		}

		for (const auto &pair: pp.servers) {
			pingpong::server *serv = pair.second;
			DBG(ansi::bold(serv->id) << " (" << serv->hostname << ")");
			for (std::shared_ptr<pingpong::channel> chan: serv->channels) {
				DBG("    " << ansi::wrap(chan->name, ansi::style::underline) << " [" << chan->mode_str() << "]");
				for (std::shared_ptr<pingpong::user> user: chan->users) {
					std::string chans = "";
					for (std::weak_ptr<pingpong::channel> user_chan: user->channels)
						chans += " " + user_chan.lock()->name;

					if (chans.empty())
						DBG("        " << user->name);
					else
						DBG("        " << user->name << ":" << chans);
				}
			}
		}
	}
}

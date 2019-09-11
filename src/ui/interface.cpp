#include <iomanip>
#include <memory>
#include <thread>

#include <csignal>

#include "ui/interface.h"
#include "lib/haunted/core/defs.h"
#include "lib/haunted/core/key.h"
#include "lib/pingpong/core/channel.h"

namespace spjalla {
	interface::interface(haunted::terminal *term): term(term) {
		using haunted::ui::boxes::box_orientation;
		using namespace haunted::ui;
		using namespace spjalla::ui;

		input     = new textinput(term);
		userbox   = new textbox(term);
		titlebar  = new label(term);
		statusbar = new label(term);

		input->set_name("input");
		userbox->set_name("userbox");
		titlebar->set_name("titlebar");
		statusbar->set_name("statusbar");

		status_window = new window("status");
		status_window->set_terminal(term);
		status_window->set_name("status_window");
		status_window->set_voffset(-1);
		active_window = status_window;
		windows.push_front(status_window);

		swappo = new boxes::swapbox(term, {}, {active_window});
		swappo->set_name("swappo");

		control *first, *second;
		if (users_side == haunted::side::left) {
			first = userbox;
			second = swappo;
		} else {
			first = swappo;
			second = userbox;
		}

		propo = new boxes::propobox(term, adjusted_ratio(), box_orientation::horizontal, first, second);
		swappo->set_parent(propo);

		expando = new boxes::expandobox(term, term->get_position(), box_orientation::vertical,
			{{titlebar, 1}, {propo, -1}, {statusbar, 1}, {input, 1}});

		propo->set_name("propo");
		expando->set_name("expando");
		term->set_root(expando);

		userbox->set_colors(ansi::color::green, ansi::color::red);
		// input->set_colors(ansi::color::magenta, ansi::color::yellow);
		titlebar->set_colors(ansi::color::blue, ansi::color::orange);
		statusbar->set_colors(ansi::color::orange, ansi::color::blue);
		active_window->set_colors(ansi::color::normal, ansi::color::normal);

		input->focus();
	}


// Private instance methods


	void interface::readjust_columns() {
		bool changed = false;
		std::vector<haunted::ui::control *> &pchildren = propo->get_children();

		if (pchildren[get_output_index()] == userbox) {
			std::swap(pchildren.at(0), pchildren.at(1));
			// haunted::ui::control *back = pchildren.back();
			// pchildren.pop_back();

			// std::swap(*active_window, *userbox);
			changed = true;
		}

		double adjusted = adjusted_ratio();

		if (propo->get_ratio() != adjusted) {
			propo->set_ratio(adjusted);
		} else if (changed) {
			// set_ratio() already draws if the ratio changed (and that's true for the preceding if block).
			propo->draw();
		}
	}

	double interface::adjusted_ratio() const {
		// It's best to avoid division by zero.
		if (users_side == haunted::side::right && users_ratio == 0.0)
			return 0.0;

		return users_side == haunted::side::right? 1.0 / users_ratio : users_ratio;
	}

	ui::window * interface::get_window(const std::string &window_name, bool create) {
		if (window_name == "status") {
			if (status_window == nullptr && create)
				status_window = new_window("status");
			return status_window;
		}

		if (swappo->empty())
			return nullptr;

		throw std::runtime_error("Unimplemented.");
	}

	ui::window * interface::get_window(pingpong::channel_ptr chan, bool create) {
		return get_window(chan->serv->hostname + " " + chan->name, create);
	}

	ui::window * interface::new_window(const std::string &name, bool append) {
		static size_t win_count = 0;
		ui::window *win = new ui::window(swappo, swappo->get_position(), name);
		win->set_name("window" + std::to_string(++win_count));
		win->set_voffset(-1);
		if (append)
			swappo->add_child(win);
		return win;
	}

	size_t interface::get_output_index() const {
		return users_side == haunted::side::left? 1 : 0;
	}


// Public instance methods


	void interface::set_users_side(haunted::side side) {
		if (users_side != side) {
			users_side = side;
			readjust_columns();
		}
	}

	void interface::set_users_ratio(double ratio) {
		if (users_ratio != ratio) {
			users_ratio = ratio;
			readjust_columns();
		}
	}

	void interface::draw() {
		term->draw();
	}

	void interface::start() {
		term->watch_size();
	}

	void interface::log(const std::string &line, ui::window *win) {
		log(haunted::ui::simpleline(line, 0), win);
	}

	void interface::log(const std::string &line, const std::string &window_name) {
		log(haunted::ui::simpleline(line, 0), window_name);
	}

	void interface::focus_window(ui::window *win) {
		if (win == nullptr)
			win = status_window;

		if (win == active_window)
			return;

		active_window->set_parent(nullptr);
		win->set_parent(propo);

		swap(*active_window, *win);
	}

	void interface::focus_window(const std::string &window_name) {
		focus_window(get_window(window_name));
	}
}

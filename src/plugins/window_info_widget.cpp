#include "pingpong/core/pputil.h"

#include "core/client.h"
#include "core/sputil.h"
#include "plugins/plugin.h"
#include "ui/status_widget.h"

#include "formicine/ansi.h"

namespace spjalla::plugins {
	class window_info_left_widget: public spjalla::ui::status_widget {
		private:
			long stamp = pingpong::util::timestamp();

		public:
			using status_widget::status_widget;

			virtual ~window_info_left_widget() {}

			std::string render(const ui::window *win, bool) const {
				if (!win) {
					return "null?";
				} else if (win->is_status()) {
					return win->window_name;
				} else if (win->is_channel()) {
					return parent->active_server_id();
				} else if (win->is_user()) {
					return parent->active_server_id();
				} else {
					return ansi::bold(win->window_name);
				}
			}
	};

	class window_info_right_widget: public spjalla::ui::status_widget {
		private:
			long stamp = pingpong::util::timestamp();

		public:
			using status_widget::status_widget;

			virtual ~window_info_right_widget() {}

			std::string render(const ui::window *win, bool) const {
				if (!win) {
					return "null?";
				} else if (win->is_status()) {
					const std::string id = parent->active_server_id();
					return id.empty()? "none"_i : id;
				} else if (win->is_channel()) {
					return util::colorize_if_dead(win->data.chan->name, win);
				} else if (win->is_user()) {
					return util::colorize_if_dead(win->data.user->name, win);
				} else {
					return "~";
				}
			}
	};

	class window_info_widget_plugin: public plugin {
		private:
			std::shared_ptr<window_info_left_widget> widget_left;
			std::shared_ptr<window_info_right_widget> widget_right;

		public:
			virtual ~window_info_widget_plugin() {}

			std::string get_name()        const override { return "Window name"; }
			std::string get_description() const override { return "Shows the name of the current window in the status"
				" bar."; }
			std::string get_version()     const override { return "0.0.0"; }

			void startup(plugin_host *host) override {
				spjalla::client *client = dynamic_cast<spjalla::client *>(host);
				if (!client) {
					DBG("Error: expected client as plugin host");
					return;
				}

				widget_left  = std::make_shared<window_info_left_widget> (client, 10);
				widget_right = std::make_shared<window_info_right_widget>(client, 15);
				client->add_status_widget(widget_left);
				client->add_status_widget(widget_right);
			}
	};
}

spjalla::plugins::window_info_widget_plugin ext_plugin {};
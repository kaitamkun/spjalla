#include "spjalla/commands/command.h"
#include "spjalla/core/client.h"

namespace spjalla::commands {
	namespace plugin {
		void info(client &cli, const int argc, const input_line &il) {
			if (argc < 2) {
				cli.get_ui().warn("Usage: " + "/plugin info "_b + "<plugin name>"_bd);
				return;
			}

			const std::string name = formicine::util::trim(il.body.substr(il.body.find("info ") + 5));
			plugins::plugin_host::plugin_tuple *tuple = cli.get_plugin(name, true);
			if (!tuple) {
				cli.get_ui().error("Couldn't find a plugin matching " + ansi::red(name) + ".");
			} else {
				plugins::plugin *plugin = std::get<1>(*tuple);
				cli.log(ansi::bold(plugin->get_name()) + "  v"_d + ansi::dim(plugin->get_version()) + "  "
					+ plugin->get_description());
			}
		}

		void list(client &cli, const int argc) {
			if (argc != 1) {
				cli.get_ui().warn("/plugin list"_b + " takes no arguments.");
				return;
			}

			const auto &plugins = cli.get_plugins();
			if (plugins.empty()) {
				cli.log("No plugins are loaded.");
			} else {
				cli.log("Plugins:");
				size_t max_name = 0, max_version = 0;

				for (const auto &tuple: plugins) {
					const size_t nlength = std::get<1>(tuple)->get_name().length();
					const size_t vlength = std::get<1>(tuple)->get_version().length();
					if (max_name < nlength)
						max_name = nlength;
					if (max_version < vlength)
						max_version = vlength;
				}

				for (const auto &tuple: plugins) {
					plugins::plugin *plugin = std::get<1>(tuple);
					const std::string name = plugin->get_name(), version = plugin->get_version(),
					                  desc = plugin->get_description();
					cli.log("    " + ansi::bold(name) + std::string(max_name - name.length(), ' ') + "  v"_d
						+ ansi::dim(version) + std::string(max_version - version.length() + 2, ' ') + desc);
				}
			}
		}
	}

	void do_plugin(client &cli, const input_line &il) {
		const size_t argc = il.args.size();
		if (argc == 0) {
			cli.log("/plugin subcommands"_u);
			cli.log("    info"_b + "  Shows information about a loaded plugin.");
			cli.log("    list"_b + "  Lists all loaded plugins.");
			cli.log("    load"_b + "  Loads a plugin by path.");
			cli.log("  unload"_b + "  Unloads a plugin by path or name.");
			return;
		}

		const std::string sub = il.first();

		if (sub == "info") {
			plugin::info(cli, argc, il);
		} else if (sub == "list") {
			plugin::list(cli, argc);
		} else {
			cli.get_ui().error("Unknown /plugin subcommand: " + ansi::red(sub));
		}
	}
}
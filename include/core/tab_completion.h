#ifndef SPJALLA_CORE_TAB_COMPLETION_H_
#define SPJALLA_CORE_TAB_COMPLETION_H_

#include <deque>
#include <string>
#include <vector>

#include "lib/haunted/core/key.h"
#include "core/input_line.h"

namespace spjalla {
	class client;
}

namespace spjalla::completions {
	/**
	 * Carries out the tab completion logic for a given input line. The raw string is passed as a reference that can
	 * be modified to add completion data. The cursor index is also passed too, and it should also be updated.
	 * For convenience, the index of the argument the cursor is in is provided, as well as the index within the
	 * argument. If either of them doesn't apply, they'll be negative.
	 */
	using completion_fn = std::function<void(client &, const input_line &, std::string &raw, size_t &cursor,
										long arg_index, long sub)>;

	void complete_command(client &, const input_line &, std::string &raw, size_t &cursor, long arg_index, long sub);

	/** Completes the /set command. */
	void complete_set(client &, const input_line &, std::string &raw, size_t &cursor, long arg_index, long sub);

	/** Contains the state data and logic for dealing with some parts of tab completion for commands. Clients keep an
	 *  instance of this and pass keypresses to it. */
	class command_completer {
		private:
			client &parent;

			/** When the user types a partial command and presses tab, it's stored here. If they press any key other
			 *  than tab, it's cleared. */
			std::string partial;

			bool has_partial = false;

		public:
			command_completer(client &parent_): parent(parent_) {}

			void on_key(const haunted::key &);
			void complete(std::string &, size_t &);
	};

	struct completion_state {
		std::deque<std::string> partials {};
		std::vector<std::function<std::string(const std::vector<std::string>)>> suggestors {};

		completion_state(const std::deque<std::string> &partials_ = {},
		const std::vector<std::function<std::string(const std::vector<std::string>)>> &suggestors_ = {}):
			partials(partials_), suggestors(suggestors_) {}
	};
}

#endif

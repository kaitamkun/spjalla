#include <iostream>
#include "spjalla/core/InputLine.h"

namespace Spjalla {
	InputLine::InputLine(const std::string &full): original(full) {
		if (full[0] == '/') {
			const size_t length = full.size();
			size_t index;
			if (1 < length) {
				for (index = 1; index < length && full[index] != ' '; ++index);

				command = formicine::util::lower(full.substr(1, index - 1));

				if (index != length) {
					body = full.substr(index + 1);

					std::string tokenize = body;
					for (size_t pos = 0; (pos = tokenize.find(' ')) != std::string::npos; tokenize.erase(0, pos + 1))
						if (pos != 0)
							args.push_back(tokenize.substr(0, pos));

					if (!tokenize.empty())
						args.push_back(tokenize);
				}
			} else minimal = true;
		} else body = full;
	}

	std::string InputLine::first() const {
		return args[0];
	}

	std::string InputLine::rest() const {
		size_t offset = body.find_first_not_of(' ') + args[0].size() + 1;
		if (body.size() < offset)
			return "";
		return body.substr(body.find_first_not_of(' ') + args[0].size() + 1);
	}

	InputLine::operator std::string() const {
		if (isCommand()) {
			std::string out = "Command[" + command + "], Body[" + body + "], Args[" + std::to_string(args.size());
			for (size_t i = 0; i < args.size(); ++i)
				out += " (" + args.at(i) + ")";
			return out + "]";
		}
		
		return "Body[" + body + "]";
	}

	std::ostream & operator<<(std::ostream &os, const InputLine &line) {
		return os << std::string(line);
	}
}

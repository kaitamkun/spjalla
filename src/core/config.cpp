#include <locale>
#include <sstream>
#include <stdexcept>

#include <cstdlib>

#include "core/config.h"
#include "core/sputil.h"

namespace spjalla {
	haunted::key keys::toggle_overlay  = {haunted::ktype::semicolon, haunted::kmod::ctrl};
	haunted::key keys::switch_server   = {haunted::ktype::x,         haunted::kmod::ctrl};
	haunted::key keys::next_window     = {haunted::ktype::n,         haunted::kmod::ctrl};
	haunted::key keys::previous_window = {haunted::ktype::p,         haunted::kmod::ctrl};


// Private static methods


	bool config::ensure_config_dir(const std::string &name) {
		std::filesystem::path config_path = util::get_home() / name;
		if (!std::filesystem::exists(config_path)) {
			std::filesystem::create_directory(config_path);
			return true;
		}

		return false;
	}

	std::pair<std::string, std::string> config::parse_kv_pair(const std::string &str) {
		if (str.empty())
			throw std::invalid_argument("Can't parse empty string as key-value pair");

		const size_t equals = str.find('=');

		if (equals == std::string::npos)
			throw std::invalid_argument("No equals sign found in key-value pair");

		if (equals == 0 || equals == str.find_first_not_of(" "))
			throw std::invalid_argument("Empty key in key-value pair");

		std::string key = str.substr(0, equals);
		util::trim(key);

		for (char ch: key) {
			if (!std::isalnum(ch))
				throw std::invalid_argument("Key isn't alphanumeric in key-value pair");
		}

		return {key, str.substr(equals + 1)};
	}

	std::string config::parse_string(const std::string &value) {
		const size_t vlength = value.length();

		// Special case: an empty value represents an empty string, same as a pair of double quotes.
		if (vlength == 0)
			return "";

		if (vlength < 2)
			throw std::invalid_argument("Invalid length of string value");

		if (value.front() != '"' || value.back() != '"')
			throw std::invalid_argument("Invalid quote placement in string value");

		return util::unescape(value.substr(1, vlength - 2));
	}


// Public static methods


	std::pair<std::string, long> config::parse_long_line(const std::string &str) {
		std::string key, value;
		std::tie(key, value) = parse_kv_pair(str);

		const char *value_cstr = value.c_str();
		char *end;
		long parsed = strtol(value_cstr, &end, 10);
		if (end != value_cstr + value.size())
			throw std::invalid_argument("Invalid value in key-value pair; expected a long");

		return {key, parsed};
	}

	std::pair<std::string, double> config::parse_double_line(const std::string &str) {
		std::string key, value;
		std::tie(key, value) = parse_kv_pair(str);

		size_t idx;
		double parsed = std::stod(value, &idx);
		if (idx != value.length())
			throw std::invalid_argument("Invalid value in key-value pair; expected a double");

		return {key, parsed};
	}

	std::pair<std::string, std::string> config::parse_string_line(const std::string &str) {
		std::string key, value;
		std::tie(key, value) = parse_kv_pair(str);

		return {key, parse_string(value)};
	}

	config::line_type config::get_line_type(const std::string &str) noexcept {
		try {
			std::string value;
			std::tie(std::ignore, value) = parse_kv_pair(str);

			if (value.empty())
				return config::line_type::string_;

			if (value.find_first_not_of("0123456789") == std::string::npos)
				return config::line_type::long_;

			if (value.find_first_not_of("0123456789.") == std::string::npos)
				return config::line_type::double_;

			if (value.size() >= 2 && value.front() == '"' && value.back() == '"')
				return config::line_type::string_;

			return config::line_type::invalid;
		} catch (const std::invalid_argument &) {
			return config::line_type::invalid;
		}
	}
}

#include <cstring>

#include "spjalla/util/BackwardReader.h"
#include "spjalla/core/Util.h"

#include "lib/formicine/ansi.h"

namespace Spjalla::Util {
	BackwardReader::BackwardReader(std::fstream &&stream_, ssize_t chunk_size_):
	                                 stream(std::move(stream_)), chunk_size(chunk_size_) {
		stream.seekg(0, std::ios::end);
		chunk = new char[chunk_size];
		buffer.reserve(chunk_size);
	}

	BackwardReader::~BackwardReader() {
		stream.close();
		delete[] chunk;
	}

	void BackwardReader::removeLastNewline(size_t &nlpos) {
		if (!buffer.empty() && buffer.back() == '\n') {
			buffer.pop_back();
			nlpos = buffer.find_last_of('\n');
		}
	}

	bool BackwardReader::readline(std::string &out) {
		if (done) {
			out = "";
			return false;
		}

		size_t nlpos = buffer.find_last_of('\n');
		std::fstream::pos_type current_pos = -1;
		while (nlpos == std::string::npos) {
			current_pos = stream.tellg();

			if (current_pos == -1L) {
				nlpos = buffer.find_last_of('\n');
				break;
			}

			if (current_pos < chunk_size) {
				stream.seekg(0, std::ios::beg);
				stream.read(chunk, current_pos);
				stream.seekg(0, std::ios::beg);
				buffer = std::string(chunk, current_pos) + buffer;
				removeLastNewline(nlpos = buffer.find_last_of('\n'));
				break;
			} else {
				stream.seekg(-chunk_size, std::ios::cur);
				stream.read(chunk, chunk_size);
				stream.seekg(-chunk_size, std::ios::cur);
				buffer = std::string(chunk, chunk_size) + buffer;
				removeLastNewline(nlpos = buffer.find_last_of('\n'));
			}
		}

		if (current_pos == 0)
			done = true;

		if (nlpos != 0 || buffer.length() != 1) {
			while (nlpos == buffer.length() - 1 && buffer.length() > 0) {
				buffer.pop_back();
				nlpos = buffer.find_last_of('\n');
			}
		}

		if (nlpos == std::string::npos) {
			out = buffer;
			return !buffer.empty();
		}

		if (nlpos == 0 && buffer[nlpos] != '\n') {
			out = buffer;
			buffer.clear();
		} else {
			out = buffer.substr(nlpos + 1);
			buffer.erase(nlpos);
		}

		return true;
	}

	size_t BackwardReader::readlines(std::vector<std::string> &out, size_t n) {
		size_t lines_read = 0;
		std::string line;

		while (readline(line) && lines_read < n) {
			++lines_read;
			out.push_back(line);
		}

		return lines_read;
	}

	void BackwardReader::reset() {
		delete[] chunk;
		chunk = new char[chunk_size];
		stream.seekg(0, std::ios::end);
		buffer.clear();
	}
}

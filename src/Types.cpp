//
// Created by Twister on 29/09/2018.
//
#include "Types.h"

#include <sstream>
#include <cctype>

namespace tgf {

	char StrReader::next() {
		if (m_pos >= m_data.size()) { return '\0'; }
		return m_data[m_pos++];
	}

	char StrReader::peek() {
		if (m_pos >= m_data.size()) { return '\0'; }
		return m_data[m_pos];
	}

	double StrReader::nextDouble() {
		std::stringstream stm;
		char c = next();
		while (std::isdigit(c) || c == '-' || c == '.' && c != '\0') {
			stm << c;
			c = next();
		}
		double ret = DBL_MIN;
		stm >> ret;
		return ret;
	}

	int StrReader::nextInt() {
		std::stringstream stm;
		char c = next();
		while (std::isdigit(c) || c == '-' && c != '\0') {
			stm << c;
			c = next();
		}
		int ret = INT_MIN;
		stm >> ret;
		return ret;
	}

	Str StrReader::nextChunk(char separator) {
		std::stringstream stm;
		char c = next();
		while (c != separator && c != '\0') {
			stm << c;
			c = next();
		}
		return stm.str();
	}

	StrReader::StrReader(const Str& data) {
		m_data = data;
		m_pos = 0;
	}

	ByteReader::ByteReader(u8* data, u32 size) {
		m_data = data;
		m_size = size;
		m_pos = 0;
	}

}

//
// Created by Twister on 26/09/2018.
//

#ifndef TGF_TYPES_H
#define TGF_TYPES_H

#include <array>
#include <map>
#include <vector>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <climits>
#include <cfloat>

using u8 = uint8_t;
using i8 = int8_t;
using u32 = uint32_t;
using i32 = int32_t;

constexpr float PI = 3.141592654f;

template <typename T> using Vec = std::vector<T>;
template <typename K, typename V> using Map = std::map<K, V>;
template <typename T, u32 S> using Array = std::array<T, S>;
template <typename T> using Ptr = std::unique_ptr<T>;
using Str = std::string;

namespace tgf {
	class StrReader {
	public:
		explicit StrReader(const Str& data);

		char next();
		char peek();
		bool hasNext() const { return m_pos < m_data.size(); }

		double nextDouble();
		int nextInt();
		Str nextChunk(char separator=' ');

	private:
		Str m_data;
		u32 m_pos;
	};

	class ByteReader {
	public:
		explicit ByteReader(u8* data, u32 size);

		bool hasNext() const { return m_pos < m_size; }

		template <typename T>
		T nextValue() {
			T val{};
			std::memcpy(&val, m_data, sizeof(T));
			m_data += sizeof(T);
			m_pos += sizeof(T);
			return val;
		}

	private:
		u8* m_data;
		u32 m_pos, m_size;
	};

}

#endif //TGF_TYPES_H

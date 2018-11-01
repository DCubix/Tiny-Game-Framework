//
// Created by Twister on 26/09/2018.
//

#include "Image.h"

#include <algorithm>

#include "Log.h"

namespace tgf {
	Image::Image(u32 w, u32 h)
		: m_width(w), m_height(h)
	{
		m_data.resize(w * h);
		std::fill(m_data.begin(), m_data.end(), 0);
	}

	u8 Image::get(i32 x, i32 y, bool repeat) {
		if (!repeat && (x < 0 || x >= m_width || y < 0 || y >= m_height)) {
			return 0;
		}
		x = x % m_width;
		y = y % m_height;
		return m_data[x + y * m_width];
	}

	void Image::set(i32 x, i32 y, u8 color) {
		if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
			return;
		}
		m_data[x + y * m_width] = color;
	}

	void Image::resize(u32 w, u32 h) {
		if (w * h == 0) {
			LogW("Invalid image size.");
			return;
		}
		m_width = w;
		m_height = h;
		m_data.resize(w * h);
		std::fill(m_data.begin(), m_data.end(), 0);
	}

}

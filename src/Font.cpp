//
// Created by Twister on 02/10/2018.
//

#include "Font.h"
#include "Log.h"

namespace tgf {

	Font::Font(Image* image, const Str& charMap, u32 rows, u32 cols) {
		LogAssert(image != nullptr, "Invalid image.");
		m_charMap = charMap;
		m_image = image;
		m_rows = rows;
		m_columns = cols;
		m_charSpacing = 1;
	}

	u32 Font::stringWidth(const Str& txt) {
		u32 cw = m_image->width() / m_columns;
		u32 w = 0;
		for (char c : txt) {
			if (c == '\n') {
				break;
			} else {
				w += cw + m_charSpacing;
			}
		}
		return w;
	}
}
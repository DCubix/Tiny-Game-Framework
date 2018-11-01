//
// Created by Twister on 02/10/2018.
//

#ifndef TGF_FONT_H
#define TGF_FONT_H

#include "Types.h"
#include "Image.h"

namespace tgf {
	class Font {
	public:
		Font() = default;
		Font(Image* image, const Str& charMap, u32 rows, u32 cols);

		u32 spacing() const { return m_charSpacing; }
		void setSpacing(u32 c) { m_charSpacing = c; }
		const Str& charMap() const { return m_charMap; }
		void setCharMap(const Str& charMap) { m_charMap = charMap; }

		Image* image() { return m_image; }
		u32 columns() const { return m_columns; }
		u32 rows() const { return m_rows; }

		u32 stringWidth(const Str& txt);
		u32 height() const { return m_image->height() / m_rows; }

	private:
		Image* m_image;
		u32 m_charSpacing, m_columns, m_rows;
		Str m_charMap;
	};
}


#endif //TGF_FONT_H

//
// Created by Twister on 26/09/2018.
//

#ifndef TGF_IMAGE_H
#define TGF_IMAGE_H

#include "Types.h"

namespace tgf {
	class Image {
		friend class Fx;
	protected:
		Image(u32 w, u32 h);

	public:
		virtual ~Image() = default;

		u8 get(i32 x, i32 y, bool repeat = false);
		void set(i32 x, i32 y, u8 color);

		void resize(u32 w, u32 h);

		u32 width() const { return m_width; }
		u32 height() const { return m_height; }
		Vec<u8>& data() { return m_data; }

	private:
		Vec<u8> m_data;
		u32 m_width, m_height;
	};
}


#endif //TGF_IMAGE_H

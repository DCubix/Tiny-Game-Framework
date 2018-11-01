//
// Created by Twister on 30/09/2018.
//

#ifndef TGF_RECT_H
#define TGF_RECT_H

#include "Types.h"
#include "Log.h"

namespace tgf {
	struct Rect {
		i32 x, y, w, h;

		Rect() : x(0), y(0), w(0), h(0) {}
		Rect(i32 x, i32 y, i32 w, i32 h) : x(x), y(y), w(w), h(h) {}
		bool valid() const { return w*h > 0; }

		Str toString() const {
			return _intern::Stringfy("(", x, ", ", y, ", ", w, ", ", h, ")");
		}
	};
}


#endif //TGF_RECT_H

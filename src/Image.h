//
// Created by Twister on 26/09/2018.
//

#ifndef TGF_IMAGE_H
#define TGF_IMAGE_H

#include "Types.h"
#include "Rect.h"

#define SOL_CHECK_ARGUMENTS 0
#define SOL_NO_CHECK_NUMBER_PRECISION 1
#include "sol.hpp"

namespace tgf {
	struct Anim {
		float time;
		float speed;
		bool loop;

		Vec<u32> frames;
		u32 currentFrame;
	};

	class Image {
		friend class Fx;
		friend class Lua;
	protected:
		Image(u32 w, u32 h);

		void update(float dt);
		void addL(const Str& name, sol::table frames);
	public:
		virtual ~Image() = default;

		u8 get(i32 x, i32 y, bool repeat = false);
		void set(i32 x, i32 y, u8 color);

		void resize(u32 w, u32 h);

		u32 width() const { return m_width; }
		u32 height() const { return m_height; }
		Vec<u8>& data() { return m_data; }

		// Animation
		Str animation() const { return m_currentAnimation; }
		Rect clip() const { return m_clip; }

		void animate(u32 rows, u32 cols);
		void add(const Str& name, const Vec<u32>& frames);
		void play(const Str& name, u32 fps, bool loop);
		void reset();
		bool animatable() const { return m_rows * m_cols > 0; }

	private:
		Vec<u8> m_data;
		u32 m_width, m_height;

		// Animation Data
		Map<Str, Anim> m_animations;
		Str m_currentAnimation;
		u32 m_rows, m_cols;
		Rect m_clip;
	};
}


#endif //TGF_IMAGE_H

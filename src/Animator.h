//
// Created by Twister on 30/09/2018.
//

#ifndef TGF_SPRITESHEET_H
#define TGF_SPRITESHEET_H

#include "Types.h"
#include "Rect.h"

namespace tgf {
	struct Anim {
		float time;
		float speed;
		bool loop;

		Vec<u32> frames;
		u32 currentFrame;
	};

	class Animator {
		friend class Lua;
	public:
		Animator() = default;
		Animator(u32 width, u32 height, u32 rows, u32 cols);

		void add(const Str& name, const Vec<u32>& frames);
		void play(const Str& name, u32 fps, bool loop);
		void reset();

		void update(float dt);

		Rect clip() { m_inUse = true; return m_clip; }
		bool inUse() const { return m_inUse; }
		Str animation() const { return m_currentAnimation; }

	private:
		Map<Str, Anim> m_animations;
		Str m_currentAnimation;
		u32 m_rows, m_cols;
		u32 m_imageWidth, m_imageHeight;
		Rect m_clip;
		bool m_inUse;
	};
}


#endif //TGF_SPRITESHEET_H

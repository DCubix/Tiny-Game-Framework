//
// Created by Twister on 26/09/2018.
//

#include "Image.h"

#include <algorithm>
#include <cmath>

#include "Log.h"

namespace tgf {
	Image::Image(u32 w, u32 h)
		: m_width(w), m_height(h)
	{
		m_data.resize(w * h);
		std::fill(m_data.begin(), m_data.end(), 0);

		// Animation
		m_rows = 0;
		m_cols = 0;
		m_currentAnimation = "";
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

	void Image::animate(u32 rows, u32 cols) {
		m_rows = rows;
		m_cols = cols;
		m_clip = Rect(0, 0, m_width, m_height);
	}

	void Image::add(const Str& name, const Vec<u32>& frames) {
		auto pos = m_animations.find(name);
		if (pos != m_animations.end()) {
			m_currentAnimation = name;
			return;
		}

		Anim anim;
		anim.currentFrame = 0;
		anim.frames = Vec<u32>(frames);
		anim.speed = 1.0f;
		anim.loop = false;
		anim.time = 0.0f;
		m_animations[name] = anim;

		if (m_currentAnimation.empty()) {
			m_currentAnimation = name;
		}
	}

	void Image::play(const Str& name, u32 fps, bool loop) {
		auto pos = m_animations.find(name);
		if (pos == m_animations.end()) {
			return;
		}

		Anim& anim = m_animations[name];
		anim.speed = 1.0f / float(fps);
		anim.loop = loop;

		m_currentAnimation = name;
	}

	void Image::reset() {
		if (m_currentAnimation.empty()) {
			return;
		}
		Anim& current = m_animations[m_currentAnimation];
		current.time = 0;
	}

	void Image::update(float dt) {
		if (m_animations.empty()) return;
		if (m_currentAnimation.empty()) return;
		if (m_rows * m_cols == 0) return;

		Anim& current = m_animations[m_currentAnimation];
		size_t maxF = current.frames.empty() ? m_rows * m_cols : current.frames.size();
		if ((current.time += dt) >= current.speed) {
			current.time = 0.0f;
			if (current.currentFrame++ >= maxF-1) {
				if (current.loop) {
					current.currentFrame = 0;
				} else {
					current.currentFrame = u32(maxF-1);
				}
			}
		}

		u32 frame = 0;
		if (current.frames.empty()) {
			frame = current.currentFrame;
		} else {
			frame = current.frames[current.currentFrame];
		}

		m_clip.w = m_width / m_cols;
		m_clip.h = m_height / m_rows;
		m_clip.x = (frame % m_cols) * m_clip.w;
		m_clip.y = std::floor(frame / m_cols) * m_clip.h;
	}

	void Image::addL(const Str& name, sol::table frames) {
		add(name, frames.as<Vec<u32>>());
	}

}

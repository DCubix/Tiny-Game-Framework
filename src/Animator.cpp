//
// Created by Twister on 30/09/2018.
//

#include "Animator.h"

#include <assert.h>
#include <cmath>

namespace tgf {

	Animator::Animator(u32 width, u32 height, u32 rows, u32 cols) {
		m_rows = rows;
		m_cols = cols;
		m_imageWidth = width;
		m_imageHeight = height;
		m_clip = Rect(0, 0, m_imageWidth, m_imageHeight);
	}

	void Animator::update(float dt) {
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

		m_clip.w = m_imageWidth / m_cols;
		m_clip.h = m_imageHeight / m_rows;
		m_clip.x = (frame % m_cols) * m_clip.w;
		m_clip.y = std::floor(frame / m_cols) * m_clip.h;

		m_inUse = false;
	}

	void Animator::add(const Str& name, const Vec<u32>& frames) {
		auto pos = m_animations.find(name);
		if (pos != m_animations.end()) {
			m_currentAnimation = name;
			return;
		}

		Anim anim;
		anim.currentFrame = 0;
		anim.frames = frames;
		anim.speed = 1.0f;
		anim.loop = false;
		anim.time = 0.0f;
		m_animations[name] = anim;

		if (m_currentAnimation.empty()) {
			m_currentAnimation = name;
		}
	}

	void Animator::play(const Str& name, u32 fps, bool loop) {
		auto pos = m_animations.find(name);
		if (pos == m_animations.end()) {
			return;
		}

		Anim& anim = m_animations[name];
		anim.speed = 1.0f / float(fps);
		anim.loop = loop;

		m_currentAnimation = name;
	}

	void Animator::reset() {
		if (m_currentAnimation.empty()) {
			return;
		}
		Anim& current = m_animations[m_currentAnimation];
		current.time = 0;
	}

}

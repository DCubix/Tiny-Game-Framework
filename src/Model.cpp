#include "Model.h"

#include "3DRendering.h"
#include "Log.h"

#include <algorithm>

namespace tgf {
	Model* Model::addAnimation(const Str& name, u32 start, u32 end) {
		ModelAnimation anim;
		anim.frame = start;
		anim.frameStart = start;
		anim.frameEnd = end;
		anim.loop = true;
		anim.speed = 1.0f / 25.0f;
		anim.time = 0.0f;

		if (m_currentAnimation.empty()) m_currentAnimation = name;

		m_animations[name] = anim;

		return this;
	}

	void Model::play(const Str& name, u32 fps, bool loop) {
		if (m_animations.find(name) == m_animations.end()) {
			return;
		}
		ModelAnimation& anim = m_animations[name];
		anim.loop = loop;
		anim.speed = 1.0f / float(fps);
		//anim.time = 0.0f;
		// anim.frame = anim.frameStart;
		m_currentAnimation = name;
	}

	ModelVertex* Model::getVertex(u32 index)  {
		if (m_interpolatedVertices.empty()) {
			return &m_vertices.begin()->second[index];
		}
		return &m_interpolatedVertices[index];
	}

	void Model::update(float dt) {
		if (m_animations.empty()) return;
		if (m_currentAnimation.empty()) return;

		ModelAnimation& current = m_animations[m_currentAnimation];
		if ((current.time += dt) >= current.speed) {
			current.time = 0;
			if (current.frame++ >= current.frameEnd-1) {
				if (current.loop) {
					current.frame = current.frameStart;
				} else {
					current.frame = current.frameEnd;
				}
			}
		}

		if (m_interpolatedVertices.empty()) {
			m_interpolatedVertices.resize(m_vertexCount);
			for (u32 i = 0; i < m_vertexCount; i++) {
				ModelVertex& v = m_interpolatedVertices[i];
				ModelVertex a = m_vertices[current.frameStart][i];
				v.x = a.x;
				v.y = a.y;
				v.z = a.z;
				v.nx = a.nx;
				v.ny = a.ny;
				v.nz = a.nz;
				v.s = a.s;
				v.t = a.t;
			}
		}

		Vec<u32> keyFrames;
		keyFrames.reserve(m_vertices.size());
		for (auto [k, v] : m_vertices) keyFrames.push_back(k);
		std::sort(keyFrames.begin(), keyFrames.end());

		u32 f = current.frame;
		for (u32 i = 0; i < keyFrames.size()-1; i++) {
			u32 ka = keyFrames[i];
			u32 kb = keyFrames[i+1];
			if (f >= ka && f <= kb) {
				float ratio = float(f - ka) / float(kb-ka);
				for (u32 i = 0; i < m_vertexCount; i++) {
					ModelVertex va = m_vertices[ka][i];
					ModelVertex vb = m_vertices[kb][i];
					ModelVertex& r = m_interpolatedVertices[i];

					r.x = glm::mix(va.x, vb.x, ratio);
					r.y = glm::mix(va.y, vb.y, ratio);
					r.z = glm::mix(va.z, vb.z, ratio);
					r.nx = glm::mix(va.nx, vb.nx, ratio);
					r.ny = glm::mix(va.ny, vb.ny, ratio);
					r.nz = glm::mix(va.nz, vb.nz, ratio);
				}
			}
		}
	}
}

#ifndef TGF_MODEL_H
#define TGF_MODEL_H

#include "Types.h"

namespace tgf {
	struct ModelVertex {
		float x, y, z, s, t;
		float nx, ny, nz;

		ModelVertex() {}
		ModelVertex(float x, float y, float z, float s, float t)
			: x(x), y(y), z(z), s(s), t(t), nx(0), ny(0), nz(0)
		{}
	};

	struct ModelAnimation {
		u32 frameStart, frameEnd;
		u32 frame;
		float speed, time;
		bool loop;
	};

	struct Face {
		u32 vi, ti;
		Face(){}
		Face(u32 vp, u32 vt) : vi(vp), ti(vt) {}
	};

	class Model {
		friend class Fx;
		friend class Lua;
	protected:
		Model() {}

		u32 m_frameCount;
		Map<Str, ModelAnimation> m_animations;
		Str m_currentAnimation;

		u32 m_vertexCount;
		Map<u32, Vec<ModelVertex>> m_vertices;
		Vec<u32> m_indices;

		Vec<ModelVertex> m_interpolatedVertices;

		u32 getIndexLua(u32 index) { return getIndex(index-1)+1; }
		ModelVertex* getVertexLua(u32 index) { return &m_interpolatedVertices[index-1]; }

		void update(float dt);

	public:
		virtual ~Model() = default;

		Map<u32, Vec<ModelVertex>>* vertices() { return &m_vertices; }
		const Vec<u32>& indices() const { return m_indices; }

		ModelVertex* getVertex(u32 index);
		u32 getIndex(u32 index) { return m_indices[index]; }
		u32 vertexCount() const { return m_vertices.size(); }
		u32 indexCount() const { return m_indices.size(); }

		Model* addAnimation(const Str& name, u32 start, u32 end);
		void play(const Str& name, u32 fps, bool loop);

	};
}

#endif // TGF_MODEL_H

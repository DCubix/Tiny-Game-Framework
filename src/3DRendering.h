//
// Created by Twister on 20/10/2018.
//
#ifndef TGF_3DRENDERING_H
#define TGF_3DRENDERING_H

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "Types.h"

#include <optional>

namespace tgf { namespace td {

	struct Vertex {
		glm::vec4 position;
		glm::vec2 uv;
		glm::vec3 normal;
		u8 color;

		Vertex transform(const glm::mat4& t) const;
		Vertex lerp(const Vertex& o, float amt) const;
		void floor();
	};

	struct Triangle {
		Vertex v0, v1, v2;
		glm::vec4 vpos0, vpos1, vpos2;
		int minX, minY, maxX, maxY;
	};

	#define T_MAX_MATRIX_TACK_DEPTH 128

	class MatrixStack {
	public:
		MatrixStack() : m_stackPointer(0) {}

		void loadIdentity();
		void pushMatrix();
		void popMatrix();

		void perspective(float fov, float aspect, float znear, float zfar);
		void ortho(float left, float right, float bottom, float top, float znear, float zfar);

		void translate(float x, float y, float z);
		void rotate(float angle, float x, float y, float z);
		void scale(float x, float y, float z);

		glm::mat4 matrix() const { return m_matrixStack[m_stackPointer]; }

	private:
		Array<glm::mat4, T_MAX_MATRIX_TACK_DEPTH> m_matrixStack;
		int m_stackPointer;
	};

	Vertex toScreenSpace(Vertex v, int width, int height);
	glm::vec3 barycentric(
		const glm::vec2& p,
		const glm::vec4& v0,
		const glm::vec4& v1,
		const glm::vec4& v2
	);

	void clipPolygonComponent(
		Vec<Vertex> vertices, int comp, float factor,
		Vec<Vertex>& out
	);

	bool clipPolygonAxis(
		std::vector<Vertex>& vertices,
		std::vector<Vertex>& aux,
		int comp
	);

	Vertex createVertex(const Vertex& v0, int drawWidth, int drawHeight);

	std::optional<Triangle> createTriangle(
		const Vertex& v0, const Vertex& v1, const Vertex& v2,
		int drawWidth, int drawHeight, const glm::mat4& modelView
	);

	float wrap(float flt, float max);

}}

#endif // TGF_3DRENDERING_H

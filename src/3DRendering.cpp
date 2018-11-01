#include "3DRendering.h"

#include <algorithm>
#include <numeric>
#include <future>
#include <cstring>
#include <cmath>
#include <vector>
#include <utility>

namespace tgf { namespace td {
	Vertex Vertex::transform(const glm::mat4& t) const {
		Vertex tvx;
		tvx.position = t * position;
		tvx.uv = uv;
		tvx.color = color;
		tvx.normal = glm::normalize(glm::vec3(t * glm::vec4(normal, 0.0f)));
		return tvx;
	}

	Vertex Vertex::lerp(const Vertex& o, float amt) const {
		Vertex tvx;
		tvx.position = glm::mix(position, o.position, amt);
		tvx.uv = glm::mix(uv, o.uv, amt);
		tvx.color = color;
		tvx.normal = glm::mix(normal, o.normal, amt);
		return tvx;
	}

	void Vertex::floor() {
		position.x = std::floor(position.x);
		position.y = std::floor(position.y);
		position.z = std::floor(position.z);
	}

	Vertex toScreenSpace(Vertex v, int width, int height) {
		v.position.x = std::floor(0.5f * width * (v.position.x + 1.0f));
		v.position.y = std::floor(0.5f * height * (v.position.y + 1.0f));
		return v;
	}

	glm::vec3 barycentric(
		const glm::vec2& p,
		const glm::vec4& v0,
		const glm::vec4& v1,
		const glm::vec4& v2
	) {
		glm::vec4 ab = v1 - v0;
		glm::vec4 ac = v2 - v0;
		glm::vec2 pa = glm::vec2(v0.x, v0.y) - p;

		glm::vec3 uv1 = glm::cross(glm::vec3(ac.x, ab.x, pa.x), glm::vec3(ac.y, ab.y, pa.y));

		if (std::abs(uv1.z) < 1e-2) {
			return glm::vec3(-1, 1, 1);
		}
		return (1.0f / uv1.z) * glm::vec3(uv1.z - (uv1.x + uv1.y), uv1.y, uv1.x);
	}

	void clipPolygonComponent(
		Vec<Vertex> vertices, int comp, float factor,
		Vec<Vertex>& out)
	{
		Vertex prevVert = vertices[vertices.size()-1];
		float prevComp = prevVert.position[comp] * factor;
		bool prevInside = prevComp <= prevVert.position.w;

		for (Vertex currVert : vertices) {
			float currComp = currVert.position[comp] * factor;
			bool currInside = currComp <= currVert.position.w;

			if (currInside ^ prevInside) {
				float lerpAmt = (prevVert.position.w - prevComp) /
						((prevVert.position.w - prevComp) - (currVert.position.w - currComp));
				out.push_back(prevVert.lerp(currVert, lerpAmt));
			}

			if (currInside) {
				out.push_back(currVert);
			}

			prevVert = currVert;
			prevComp = currComp;
			prevInside = currInside;
		}
	}

	bool clipPolygonAxis(
		Vec<Vertex>& vertices,
		Vec<Vertex>& aux,
		int comp
	)
	{
		clipPolygonComponent(vertices, comp, 1.0f, aux);
		vertices.clear();

		if (aux.empty()) {
			return false;
		}

		clipPolygonComponent(aux, comp, -1.0f, vertices);
		aux.clear();

		return !vertices.empty();
	}

	float wrap(float flt, float max) {
		if (flt > max) {
			flt -= max;
		}
		if (flt < 0.0f) {
			flt += max;
		}
		return flt;
	}

	Vertex createVertex(const Vertex& v0, int drawWidth, int drawHeight) {
		Vertex nv = v0;

		/// Perspective Divide
		nv.position /= v0.position.w;

		nv = toScreenSpace(nv, drawWidth, drawHeight);
		nv.floor();

		return nv;
	}

	std::optional<Triangle> createTriangle(
		const Vertex& v0, const Vertex& v1, const Vertex& v2,
		int drawWidth, int drawHeight, const glm::mat4& modelView
	) {
		Triangle tri;
		tri.v0 = v0;
		tri.v1 = v1;
		tri.v2 = v2;

		tri.vpos0 = v0.position;
		tri.vpos1 = v1.position;
		tri.vpos2 = v2.position;

		/// Perspective Divide
		tri.v0.position /= v0.position.w;
		tri.v1.position /= v1.position.w;
		tri.v2.position /= v2.position.w;

		Vertex vt0 = tri.v0;
		Vertex vt1 = tri.v1;
		Vertex vt2 = tri.v2;

		glm::vec4 _vs1 = vt1.position - vt0.position;
		glm::vec4 _vs2 = vt2.position - vt0.position;

		/// Cull
		glm::vec3 eye = glm::vec3(modelView[3]);
		glm::vec3 tV = glm::normalize(glm::vec3(vt0.position) - eye);
		glm::vec3 tN = glm::cross(glm::vec3(_vs1), glm::vec3(_vs2));

		if (glm::dot(tN, tV) >= 1.0f) {
			return {};
		}

		/// To screen space
		vt0 = toScreenSpace(vt0, drawWidth, drawHeight);
		vt1 = toScreenSpace(vt1, drawWidth, drawHeight);
		vt2 = toScreenSpace(vt2, drawWidth, drawHeight);

		tri.maxX = std::max(vt0.position.x, std::max(vt1.position.x, vt2.position.x));
		tri.minX = std::min(vt0.position.x, std::min(vt1.position.x, vt2.position.x));
		tri.maxY = std::max(vt0.position.y, std::max(vt1.position.y, vt2.position.y));
		tri.minY = std::min(vt0.position.y, std::min(vt1.position.y, vt2.position.y));

		vt0.floor();
		vt1.floor();
		vt2.floor();

		tri.v0 = vt0;
		tri.v1 = vt1;
		tri.v2 = vt2;

		return std::make_optional(tri);
	}

	void MatrixStack::loadIdentity() {
		m_matrixStack[m_stackPointer] = glm::mat4(1.0f);
	}

	void MatrixStack::pushMatrix() {
		if (m_stackPointer < (T_MAX_MATRIX_TACK_DEPTH - 1)) {
			m_stackPointer++;
			m_matrixStack[m_stackPointer] = m_matrixStack[m_stackPointer - 1];
		}
	}

	void MatrixStack::popMatrix() {
		if (m_stackPointer > 0)
			m_stackPointer--;
	}

	void MatrixStack::translate(float x, float y, float z) {
		glm::vec3 t(x, y, z);
		glm::mat4 tmp = m_matrixStack[m_stackPointer],
				trl = glm::translate(glm::mat4(1.0f), t);
		m_matrixStack[m_stackPointer] = tmp * trl;
	}

	void MatrixStack::rotate(float angle, float x, float y, float z) {
		glm::vec3 axis(x, y, z);
		glm::mat4 tmp = m_matrixStack[m_stackPointer],
				rot = glm::rotate(glm::mat4(1.0f), angle, axis);
		m_matrixStack[m_stackPointer] = tmp * rot;
	}

	void MatrixStack::scale(float x, float y, float z) {
		glm::vec3 s(x, y, z);
		glm::mat4 tmp = m_matrixStack[m_stackPointer],
				scl = glm::scale(glm::mat4(1), s);
		m_matrixStack[m_stackPointer] = tmp * scl;
	}

	void MatrixStack::perspective(float fov, float aspect, float znear, float zfar) {
		glm::mat4 tmp = m_matrixStack[m_stackPointer],
				pmt = glm::perspective(fov, aspect, znear, zfar);
		m_matrixStack[m_stackPointer] = tmp * pmt;
	}

	void MatrixStack::ortho(float left, float right, float bottom, float top, float znear, float zfar) {
		glm::mat4 tmp = m_matrixStack[m_stackPointer],
				pmt = glm::ortho(left, right, bottom, top, znear, zfar);
		m_matrixStack[m_stackPointer] = tmp * pmt;
	}

}}

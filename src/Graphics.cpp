//
// Created by Twister on 26/09/2018.
//

#include "Graphics.h"
#include "Fx.h"

#include <cmath>
#include <cctype>

namespace tgf {

	Graphics::Graphics(Image* target) {
		assert(target != nullptr && "Invalid target.");
		m_target = target;
		m_clip[0] = 0;
		m_clip[1] = 0;
		m_clip[2] = m_target->width();
		m_clip[3] = m_target->height();
		m_transparencyKey = -1;

		/// 3D Rendering
		m_depth.resize(target->width() * target->height());
		std::fill(m_depth.begin(), m_depth.end(), 0.0f);
		m_triangleColor = 4;
		m_boundTexture = nullptr;

		float aspect = float(target->width()) / float(target->height());
		m_projection.loadIdentity();
		m_projection.perspective(glm::radians(50.0f), aspect, 0.01f, 100.0f);
		m_modelView.loadIdentity();
		m_polygonMode = PolygonMode::Triangles;
		///

		for (u8 i = 0; i < Fx::instance().palette().size(); i++){
			m_colorRemap[i] = i;
		}

		m_matrixStack = &m_modelView;
		m_lightDirection = glm::vec3(-1.0f);
		m_lightingEnabled = false;
		m_smoothShading = false;
	}

	bool Graphics::pixel(i32 x, i32 y, u8 color) {
		if (pointInClip(x, y) &&
			(m_transparencyKey == -1 || m_transparencyKey != int(color)) &&
			color != 0xFF)
		{
			m_target->set(x, y, m_colorRemap[color]);

			return true;
		}
		return false;
	}

	void Graphics::line(i32 x1, i32 y1, i32 x2, i32 y2, u8 color) {
		int dx = std::abs(x2 - x1);
		int sx = x1 < x2 ? 1 : -1;
		int dy = -std::abs(y2 - y1);
		int sy = y1 < y2 ? 1 : -1;
		int err = dx + dy;
		int e2 = 0;

		int x = x1;
		int y = y1;

		while (true) {
			pixel(x, y, color);

			if (x == x2 && y == y2) break;
			e2 = 2 * err;
			if (e2 >= dy) { err += dy; x += sx; }
			if (e2 <= dx) { err += dx; y += sy; }
		}
	}

	bool Graphics::pointInClip(i32 x, i32 y) {
		return x >= m_clip[0] &&
		       x < m_clip[2] &&
		       y >= m_clip[1] &&
		       y < m_clip[3];
	}

	void Graphics::rect(i32 x, i32 y, u32 w, u32 h, u8 color, bool fill) {
		if (fill) {
			for (i32 ry = 0; ry < h; ry++) {
				line(x, ry+y, x+w, ry+y, color);
			}
		} else {
			line(x, y, x+w, y, color);
			line(x+w, y, x+w, y+h, color);
			line(x+w, y+h, x, y+h, color);
			line(x, y+h, x, y, color);
		}
	}

	void Graphics::circle(i32 x1, i32 y1, u32 radius, u8 color, bool fill) {
		int x = radius-1;
		int y = 0;
		int dx = 1;
		int dy = 1;
		int err = dx - (radius << 1);

		while (x >= y) {
			if (fill) {
				line(x1 - y, y1 + x, x1 + y, y1 + x, color);
				line(x1 - x, y1 + y, x1 + x, y1 + y, color);
				line(x1 - x, y1 - y, x1 + x, y1 - y, color);
				line(x1 - y, y1 - x, x1 + y, y1 - x, color);
			} else {
				pixel(x1 + x, y1 + y, color);
				pixel(x1 + y, y1 + x, color);
				pixel(x1 - y, y1 + x, color);
				pixel(x1 - x, y1 + y, color);
				pixel(x1 - x, y1 - y, color);
				pixel(x1 - y, y1 - x, color);
				pixel(x1 + y, y1 - x, color);
				pixel(x1 + x, y1 - y, color);
			}

			if (err <= 0) {
				y++;
				err += dy;
				dy += 2;
			}

			if (err > 0) {
				x--;
				dx += 2;
				err += dx - (radius << 1);
			}
		}
	}

	void Graphics::clip(i32 x1, i32 y1, i32 x2, i32 y2) {
		if (x1 * y1 + x2 * y2 == 0) {
			m_clip[0] = 0;
			m_clip[1] = 0;
			m_clip[2] = m_target->width();
			m_clip[3] = m_target->height();
			return;
		}
		m_clip[0] = x1;
		m_clip[1] = y1;
		m_clip[2] = x2;
		m_clip[3] = y2;
	}

	void Graphics::unclip() {
		clip(0, 0, 0, 0);
	}

	void Graphics::spriteAll(Image* img, i32 x, i32 y, bool flipX, bool flipY, Rect clip) {
		assert(img != nullptr && "Invalid image.");

		if (img->animatable()) {
			clip = img->clip();
		}

		i32 srcX = clip.valid() ? clip.x : 0;
		i32 srcY = clip.valid() ? clip.y : 0;
		i32 srcW = clip.valid() ? clip.w : img->width();
		i32 srcH = clip.valid() ? clip.h : img->height();

		for (i32 sy = srcY; sy < srcH+srcY; sy++) {
			for (i32 sx = srcX; sx < srcW+srcX; sx++) {
				i32 ssx = sx - srcX;
				i32 ssy = sy - srcY;
				i32 tx = i32(flipX ? srcW - 1 - ssx : ssx) + x,
					ty = i32(flipY ? srcH - 1 - ssy : ssy) + y;
				pixel(tx, ty, img->get(sx, sy, true));
			}
		}
	}

	void Graphics::sprite(Image* img, i32 x, i32 y, bool flipX, bool flipY) {
		spriteAll(img, x, y, flipX, flipY, Rect());
	}

	void Graphics::tile(Image* img, u32 index, i32 x, i32 y, i32 tileW, i32 tileH) {
		assert(img != nullptr && "Invalid image.");
		const i32 cols = img->width() / tileW;

		i32 cx = (index % cols) * tileW;
		i32 cy = (index / cols) * tileH;

		spriteAll(img, x, y, false, false, Rect(cx, cy, tileW, tileH));
	}

	void Graphics::clear(u8 color) {
		rect(0, 0, m_target->width(), m_target->height(), color, true);
		std::fill(m_depth.begin(), m_depth.end(), 0.0f);
	}


	void Graphics::remap(u8 oldColor, u8 newColor) {
		m_colorRemap[oldColor] = newColor;
	}

	void Graphics::remapNonTransparent(u8 newColor) {
		for (u8 i = 0; i < Fx::instance().palette().size(); i++){
			if (i != m_transparencyKey) {
				m_colorRemap[i] = newColor;
			}
		}
	}

	void Graphics::remapReset() {
		for (u8 i = 0; i < Fx::instance().palette().size(); i++){
			m_colorRemap[i] = i;
		}
	}

	Vector Graphics::text(Font font, const Str& text, i32 x, i32 y) {
		Vector pos(x, y);

		const u32 charW = font.image()->width() / font.columns();
		const u32 charH = font.image()->height() / font.rows();

		for (char c : text) {
			if (c == '\n') {
				pos.x = x;
				pos.y += charH + 1;
			} else if (c == ' ') {
				pos.x += charW + font.spacing();
			} else {
				size_t index = font.charMap().find_first_of(c);
				if (index != Str::npos) {
					i32 cx = (i32(index) % font.columns()) * charW;
					i32 cy = (i32(index) / font.columns()) * charH;
					spriteAll(
							font.image(),
							i32(pos.x), i32(pos.y), false, false,
							Rect(cx, cy, charW, charH)
					);
				}
				pos.x += charW + font.spacing();
			}
		}

		return pos;
	}

	/// 3D Rendering
	void Graphics::triangleUC(const Vertex& v0, const Vertex& v1, const Vertex& v2) {
		const u32 drawWidth = m_target->width();
		const u32 drawHeight = m_target->height();

		auto&& otri = createTriangle(
			v0, v1, v2,
			drawWidth, drawHeight, m_modelView.matrix()
		);
		if (!otri.has_value()) {
			return;
		}

		Triangle tri = otri.value();

		const float BX = 1.0f / drawWidth;
		const float BY = 1.0f / drawHeight;

		for (int x = tri.minX; x < tri.maxX; x++) {
			for (int y = tri.minY; y < tri.maxY; y++) {
				int i = x + y * target()->width();
				if (i < 0 || i >= target()->width() * target()->height()) {
					continue;
				}

				glm::vec3 bc = barycentric(
					glm::vec2(x, y),
					tri.v0.position,
					tri.v1.position,
					tri.v2.position
				);

				if (bc.x < -BX || bc.y < -BY || bc.z < 0.0f) { continue; }

				glm::vec3 P = glm::vec3(
					bc.x / tri.vpos0.w,
					bc.y / tri.vpos1.w,
					bc.z / tri.vpos2.w
				);
				float d = (P.x + P.y + P.z);
				P = (1.0f / d) * P;

				float z = d / 3.0f;

				if (m_depth[i] < z) {
					glm::vec2 uv = P.x * tri.v0.uv + P.y * tri.v1.uv + P.z * tri.v2.uv;
					uv.x = wrap(uv.x, 1.0f);
					uv.y = wrap(uv.y, 1.0f);

					u8 color = tri.v0.color;

					if (m_lightingEnabled) {
						// Perform lighting
						glm::vec3 tN(0.0f);
						if (!m_smoothShading) {
							glm::vec3 e1 = glm::vec3(tri.vpos1 - tri.vpos0);
							glm::vec3 e2 = glm::vec3(tri.vpos2 - tri.vpos0);
							tN = glm::normalize(glm::cross(e1, e2));
						} else {
							tN = glm::normalize(P.x * tri.v0.normal + P.y * tri.v1.normal + P.z * tri.v2.normal);
						}

						float shading = calculateShading(tN, -m_lightDirection);
						shading = glm::clamp(shading + 0.2f, 0.0f, 1.0f);

						RGB a = Fx::instance().palette()[color];
						if (m_boundTexture != nullptr) {
							i32 tx = m_boundTexture->width() * uv.x;
							i32 ty = m_boundTexture->height() * uv.y;
							a = Fx::instance().palette()[m_boundTexture->get(tx, ty, true)];
						}
						RGB scol = {
							glm::clamp(u8((float(a.r)/255.0f * shading)*255), u8(0), u8(255)),
							glm::clamp(u8((float(a.g)/255.0f * shading)*255), u8(0), u8(255)),
							glm::clamp(u8((float(a.b)/255.0f * shading)*255), u8(0), u8(255))
						};

						scol.r = Fx::instance().dither(x, y, scol.r, DitherMode::Mode8x8);
						scol.g = Fx::instance().dither(x, y, scol.g, DitherMode::Mode8x8);
						scol.b = Fx::instance().dither(x, y, scol.b, DitherMode::Mode8x8);
						color = Fx::instance().closestPaletteColor(scol);
					} else {
						if (m_boundTexture != nullptr) {
							i32 tx = m_boundTexture->width() * uv.x;
							i32 ty = m_boundTexture->height() * uv.y;
							color = m_boundTexture->get(tx, ty, true);
						}
					}

					pixel(x, y, color);
					m_depth[i] = z;
				}
			}
		}
	}

	void Graphics::triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2) {
		const glm::mat4 mvp = m_projection.matrix() * m_modelView.matrix();

		Vec<Vertex> vertices, aux;
		vertices.insert(vertices.end(), {
			v0.transform(mvp),
			v1.transform(mvp),
			v2.transform(mvp)
		});

		if (clipPolygonAxis(vertices, aux, 0) &&
			clipPolygonAxis(vertices, aux, 1) &&
			clipPolygonAxis(vertices, aux, 2))
		{
			Vertex initial = vertices[0];
			for (int i = 1; i < vertices.size() - 1; i++) {
				triangleUC(initial, vertices[i], vertices[i + 1]);
			}
		}
	}

	void Graphics::line3D(const Vertex& v0, const Vertex& v1) {
		const glm::mat4 mvp = m_projection.matrix() * m_modelView.matrix();
		const u32 drawWidth = m_target->width();
		const u32 drawHeight = m_target->height();

		Vertex vt0 = createVertex(v0.transform(mvp), drawWidth, drawHeight);
		Vertex vt1 = createVertex(v1.transform(mvp), drawWidth, drawHeight);

		line(
			vt0.position.x, vt0.position.y,
			vt1.position.x, vt1.position.y,
			m_triangleColor
		);
	}

	float Graphics::calculateShading(const glm::vec3& N, const glm::vec3& L) {
		return glm::clamp(glm::dot(N, L), 0.0f, 1.0f);
	}

	void Graphics::begin() {
		m_polygonMode = PolygonMode::Triangles;
		m_vertices.clear();
	}

	void Graphics::beginMode(const Str& mode) {
		begin();

		static const Str NAMES[] = {
			"triangles",
			"wireframe",
			"lines",
			"points"
		};
		auto beg = std::begin(NAMES);
		auto end = std::end(NAMES);
		auto it = std::find_if(beg, end,
				[&mode](const std::string& s) {
					return std::equal(s.cbegin(), s.cend(), mode.cbegin(), mode.cend(),
							[](auto c1, auto c2) {
								return std::tolower(c2) == c1;
							}
					);
				}
		);

		if (it != end) {
			int i = int(it - beg);
			m_polygonMode = PolygonMode(i);
		}
	}

	void Graphics::vertex(float x, float y, float z, float s, float t) {
		vertexN(x, y, z, s, y, 0, 0, 0);
	}

	void Graphics::vertexN(float x, float y, float z, float s, float t, float nx, float ny, float nz) {
		Vertex v;
		v.position.x = x;
		v.position.y = y;
		v.position.z = z;
		v.position.w = 1.0f;
		v.uv.x = s;
		v.uv.y = t;
		v.color = m_triangleColor;
		v.normal.x = nx;
		v.normal.y = ny;
		v.normal.z = nz;
		m_vertices.push_back(v);
	}

	void Graphics::end() {
		switch (m_polygonMode) {
			case PolygonMode::Triangles: {
				for (u32 i = 0; i < m_vertices.size(); i+=3) {
					Vertex v0 = m_vertices[i + 0];
					Vertex v1 = m_vertices[i + 1];
					Vertex v2 = m_vertices[i + 2];
					triangle(v0, v1, v2);
				}
			} break;
			case PolygonMode::Wireframe: {
				for (u32 i = 0; i < m_vertices.size(); i+=3) {
					Vertex v0 = m_vertices[i + 0];
					Vertex v1 = m_vertices[i + 1];
					Vertex v2 = m_vertices[i + 2];
					line3D(v0, v1);
					line3D(v1, v2);
					line3D(v2, v0);
				}
			} break;
			case PolygonMode::Lines: {
				for (u32 i = 0; i < m_vertices.size(); i+=2) {
					Vertex v0 = m_vertices[i + 0];
					Vertex v1 = m_vertices[i + 1];
					line3D(v0, v1);
				}
			} break;
			case PolygonMode::Points: {
				const glm::mat4 mvp = m_projection.matrix() * m_modelView.matrix();
				const u32 drawWidth = m_target->width();
				const u32 drawHeight = m_target->height();
				for (u32 i = 0; i < m_vertices.size(); i++) {
					Vertex v0 = m_vertices[i + 0].transform(mvp);
					Vertex p = createVertex(v0, drawWidth, drawHeight);
					pixel(i32(p.position.x), i32(p.position.y), m_triangleColor);
				}
			} break;
		}

	}

	void Graphics::bind(Image* img) {
		m_boundTexture = img;
	}

	void Graphics::model(Model* m) {
		for (u32 i = 0; i < m->indexCount(); i+=3) {
			u32 i0 = m->getIndex(i);
			u32 i1 = m->getIndex(i+1);
			u32 i2 = m->getIndex(i+2);
			ModelVertex v0 = *m->getVertex(i0);
			ModelVertex v1 = *m->getVertex(i1);
			ModelVertex v2 = *m->getVertex(i2);
			vertexN(v0.x, v0.y, v0.z, v0.s, v0.t, v0.nx, v0.ny, v0.nz);
			vertexN(v1.x, v1.y, v1.z, v1.s, v1.t, v1.nx, v1.ny, v1.nz);
			vertexN(v2.x, v2.y, v2.z, v2.s, v2.t, v2.nx, v2.ny, v2.nz);
		}
	}

	void Graphics::matrixMode(const Str& mode) {
		static const Str NAMES[] = {
			"modelview",
			"projection"
		};
		auto beg = std::begin(NAMES);
		auto end = std::end(NAMES);
		auto it = std::find_if(beg, end,
				[&mode](const std::string& s) {
					return std::equal(s.cbegin(), s.cend(), mode.cbegin(), mode.cend(),
							[](auto c1, auto c2) {
								return std::tolower(c2) == c1;
							}
					);
				}
		);

		if (it != end) {
			int i = int(it - beg);
			switch (i) {
				default:
				case 0: m_matrixStack = &m_modelView; break;
				case 1: m_matrixStack = &m_projection; break;
			}
		}
	}

	void Graphics::pushMatrix() {
		m_matrixStack->pushMatrix();
	}

	void Graphics::popMatrix() {
		m_matrixStack->popMatrix();
	}

	void Graphics::identity() {
		m_matrixStack->loadIdentity();
	}

	void Graphics::translate(float x, float y, float z) {
		m_matrixStack->translate(x, y, z);
	}

	void Graphics::rotate(float rot, float x, float y, float z) {
		m_matrixStack->rotate(rot, x, y, z);
	}

	void Graphics::scale(float x, float y, float z) {
		m_matrixStack->scale(x, y, z);
	}

	void Graphics::perspective(float fov, float aspect, float n, float f) {
		m_matrixStack->perspective(fov, aspect, n, f);
	}

	void Graphics::ortho(float left, float right, float bottom, float top, float n, float f) {
		m_matrixStack->ortho(left, right, bottom, top, n, f);
	}


}

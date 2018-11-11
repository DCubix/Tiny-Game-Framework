//
// Created by Twister on 26/09/2018.
//

#ifndef TGF_GRAPHICS_H
#define TGF_GRAPHICS_H

#include <assert.h>
#include <algorithm>

#include "Image.h"
#include "Rect.h"
#include "Font.h"
#include "Vector.h"

/// 3D Rendering
#include "3DRendering.h"
#include "Model.h"

namespace tgf {
	using namespace td;

	struct RGB {
		union {
			u8 comps[3];
			struct { u8 r, g, b; };
		};
	};

	enum PolygonMode {
		Triangles = 0,
		Wireframe,
		Lines,
		Points
	};

	class Graphics {
		friend class Fx;
		friend class Lua;
	public:
		Graphics() = default;
		explicit Graphics(Image* target);
		virtual ~Graphics() = default;

		void clear(u8 color);

		bool pixel(i32 x, i32 y, u8 color);
		void line(i32 x1, i32 y1, i32 x2, i32 y2, u8 color);
		void rect(i32 x, i32 y, u32 w, u32 h, u8 color, bool fill = false);
		void circle(i32 x1, i32 y1, u32 radius, u8 color, bool fill = false);

		void sprite(Image* img, i32 x, i32 y, bool flipX, bool flipY);
		void tile(Image* img, u32 index, i32 x, i32 y, i32 tileW, i32 tileH);
		Vector text(Font font, const Str& text, i32 x, i32 y);

		/// 3D Rendering
		void begin();
		void beginMode(const Str& mode);
		void color(u8 col) { m_triangleColor = col; }
		void vertex(float x, float y, float z, float s, float t);
		void model(Model* m);
		void end();
		void bind(Image* img);
		void lighting(bool enable) { m_lightingEnabled = enable; }
		void smooth(bool enable) { m_smoothShading = enable; }
		void lightDirection(float x, float y, float z) { m_lightDirection = glm::vec3(x, y, z); }

		/// Expose matrix functions
		void matrixMode(const Str& mode);
		void pushMatrix();
		void popMatrix();
		void identity();
		void translate(float x, float y, float z);
		void rotate(float rot, float x, float y, float z);
		void scale(float x, float y, float z);
		void perspective(float fov, float aspect, float n, float f);
		void ortho(float left, float right, float bottom, float top, float n, float f);
		///

		void clip(i32 x1, i32 y1, i32 x2, i32 y2);
		void unclip();

		Image* target() { return m_target; }
		void setTarget(Image* img) {
			assert(img != nullptr);
			m_target = img;
			m_depth.resize(m_target->width() * m_target->height());
			std::fill(m_depth.begin(), m_depth.end(), 0.0f);
		}

		void transparency(u8 color) { m_transparencyKey = int(color); }
		void resetTransparency() { m_transparencyKey = -1; }

		void remap(u8 oldColor, u8 newColor);
		void remapNonTransparent(u8 newColor);
		void remapReset();

	protected:
		Image* targetLua() const { return m_target; }

		void spriteAll(Image* img, i32 x, i32 y, bool flipX, bool flipY, Rect clip);
		void spriteFX(Image* img, i32 x, i32 y, bool flipX) { sprite(img, x, y, flipX, false); }
		void spriteNF(Image* img, i32 x, i32 y) { sprite(img, x, y, false, false); }

		void vertexN(float x, float y, float z, float s, float t, float nx, float ny, float nz);

	private:
		Array<u8, 16> m_colorRemap;
		Image* m_target;
		int m_clip[4];
		int m_transparencyKey;

		bool pointInClip(i32 x, i32 y);

		/// 3D Rendering
		MatrixStack m_modelView;
		MatrixStack m_projection;
		MatrixStack* m_matrixStack;
		Vec<float> m_depth;
		Image* m_boundTexture;
		glm::vec3 m_lightDirection;
		bool m_lightingEnabled, m_smoothShading;
		PolygonMode m_polygonMode;
		///

		Vec<Vertex> m_vertices;
		u8 m_triangleColor;

		void triangleUC(const Vertex& v0, const Vertex& v1, const Vertex& v2);
		void triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2);
		void line3D(const Vertex& v0, const Vertex& v1);
	};
}


#endif //TGF_GRAPHICS_H

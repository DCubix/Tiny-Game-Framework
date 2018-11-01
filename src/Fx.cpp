//
// Created by Twister on 26/09/2018.
//

#include "Fx.h"
#include "Log.h"
#include "Lua.h"
#include "stb/stb_image.h"
#include "DefaultFont.h"
#include "tmd.h"

#include <memory>
#include <algorithm>
#include <tuple>
#include <fstream>
#include <float.h>

namespace tgf {

	static const double DITHER_2x2[] = {
			0, 2,
			3, 1
	};

	static const double DITHER_3x3[] = {
			0, 7, 3,
			6, 5, 2,
			4, 1, 8
	};

	static const double DITHER_4x4[] = {
			0, 8, 2, 10,
			12, 4, 14, 6,
			3, 11, 1, 9,
			15, 7, 13, 5
	};

	static const double DITHER_8x8[] = {
			0, 32, 8, 40, 2, 34, 10, 42,
			48, 16, 56, 24, 50, 18, 58, 26,
			12, 44, 4, 36, 14, 46, 6, 38,
			60, 28, 52, 20, 62, 30, 54, 22,
			3, 35, 11, 43, 1, 33, 9, 41,
			51, 19, 59, 27, 49, 17, 57, 25,
			15, 47, 7, 39, 13, 45, 5, 37,
			63, 31, 55, 23, 61, 29, 53, 21
	};

	static const Map<u8, Vec<u32>> KEY_MAP = {
			{ ButtonLeft, { SDLK_LEFT, SDLK_s } },
			{ ButtonRight, { SDLK_RIGHT, SDLK_f } },
			{ ButtonUp, { SDLK_UP, SDLK_e } },
			{ ButtonDown, { SDLK_DOWN, SDLK_d } },
			{ ButtonX, { SDLK_z, SDLK_c, SDLK_n, SDLK_KP_MINUS, SDLK_LSHIFT, SDLK_TAB } },
			{ ButtonY, { SDLK_x, SDLK_v, SDLK_m, SDLK_8, SDLK_a, SDLK_q } },
			{ ButtonSelect, { SDLK_SPACE, SDLK_RETURN, SDLK_KP_ENTER } }
	};

	static std::tuple<bool, u8> keyMapGetButton(u32 sdlKey) {
		for (auto&& e : KEY_MAP) {
			auto pos = std::find(e.second.begin(), e.second.end(), sdlKey);
			if (pos != e.second.end()) {
				return std::make_tuple(true, e.first);
			}
		}
		return std::make_tuple(false, 0);
	}

	Ptr<Fx> Fx::s_instance;

	Fx::~Fx() {
		SDL_DestroyTexture(m_backBuffer);
		SDL_DestroyRenderer(m_renderer);
		SDL_DestroyWindow(m_window);
		SDL_Quit();
	}

	void Fx::init(const Str& title, u32 width, u32 height, u8 scale) {
		if (SDL_Init(SDL_INIT_EVERYTHING) > 0) {
			LogE(SDL_GetError());
			return;
		}

		scale = std::min(scale, u8(4));
		m_width = width * scale;
		m_height = height * scale;
		m_scale = scale;

		m_window = SDL_CreateWindow(
				title.c_str(),
				SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
				width * scale, height * scale,
				SDL_WINDOW_SHOWN
		);
		if (!m_window) {
			LogE(SDL_GetError());
			SDL_Quit();
			return;
		}

		m_renderer = SDL_CreateRenderer(m_window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		if (!m_renderer) {
			LogE(SDL_GetError());
			SDL_Quit();
			return;
		}

		m_audioSystem = std::make_unique<AudioSystem>();

		m_screenBuffer = Ptr<Image>(new Image(width, height));
		m_graphics = std::make_unique<Graphics>(m_screenBuffer.get());

		m_backBuffer = SDL_CreateTexture(
				m_renderer,
				SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,
				width, height
		);

		/// Default color palette
		const RGB pal[] = {
			{ 15, 21, 27 },
			{ 41, 39, 50 },
			{ 83, 88, 103 },
			{ 149, 146, 143 },
			{ 241, 241, 234 },
			{ 197, 141, 101 },
			{ 141, 82, 66 },
			{ 81, 61, 61 },
			{ 236, 213, 109 },
			{ 234, 119, 48 },
			{ 205, 61, 61 },
			{ 124, 63, 140 },
			{ 48, 66, 113 },
			{ 0, 131, 200 },
			{ 71, 164, 77 },
			{ 31, 97, 67 }
		};

		std::copy(std::begin(pal), std::end(pal), m_colorPalette.begin());

		m_defaultFont = std::make_unique<Font>(
				loadImageFromMemory(font_data, font_size, DitherMode::None),
				"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-?!'\":;/\\)(,.$_+*=",
				5, 16
		);
		m_defaultFont->setSpacing(0);
	};

	void Fx::flip() {
		Uint8* pixels;
		int pitch;
		SDL_LockTexture(m_backBuffer, nullptr, (void**)&pixels, &pitch);
		for (u32 y = 0; y < height(); y++) {
			for (u32 x = 0; x < width(); x++) {
				u32 i = x + y * width();
				u32 j = i * 3;
				u8 ci = m_screenBuffer->get(x, y);
				RGB col = m_colorPalette[ci];
				pixels[j + 0] = col.r;
				pixels[j + 1] = col.g;
				pixels[j + 2] = col.b;
			}
		}
		SDL_UnlockTexture(m_backBuffer);

		SDL_RenderClear(m_renderer);

		SDL_Rect dst = { 0, 0, int(m_width), int(m_height) };
		SDL_RenderCopy(m_renderer, m_backBuffer, nullptr, &dst);

		SDL_RenderPresent(m_renderer);
	}

	u8 Fx::color(const Str& name) {
		static const Str NAMES[16] = {
				"black",
				"darkgray",
				"gray",
				"lightgray",
				"white",
				"lightbrown",
				"brown",
				"darkbrown",
				"yellow",
				"orange",
				"red",
				"purple",
				"darkblue",
				"blue",
				"green",
				"darkgreen"
		};

		auto beg = std::begin(NAMES);
		auto end = std::end(NAMES);
		auto it = std::find_if(beg, end,
				[&name](const std::string& s) {
					return std::equal(s.cbegin(), s.cend(), name.cbegin(), name.cend(),
							[](auto c1, auto c2) {
								return std::tolower(c2) == c1;
							}
					);
				}
		);

		if (it != end) {
			return u8(it - beg);
		}
		return 0;
	}

	int Fx::run(Game* game) {
#define TIME() (double(SDL_GetTicks()) / 1000.0)
		if (game == nullptr) {
			LogE("Game should not be null.");
			return 1;
		}

		m_running = true;

		m_game = Ptr<Game>(game);
		m_game->create();

		double lastTime = TIME();
		double accum = 0.0;

		SDL_Event evt;

		while (true) {
			if (!m_running) break;

			bool canRender = false;
			double current = TIME();
			double delta = current - lastTime;
			lastTime = current;
			accum += delta;

			while (accum >= TGF_TIMESTEP) {
				accum -= TGF_TIMESTEP;
				updateInput(evt);
				updateAnimators(float(TGF_TIMESTEP));
				m_game->update(float(TGF_TIMESTEP));
				canRender = true;
			}

			if (canRender) {
				m_game->draw(*m_graphics.get());
			}
		}

#undef TIME
		return 0;
	}

	Fx& Fx::instance() {
		if (!s_instance) {
			s_instance = Ptr<Fx>(new Fx());
		}
		return *s_instance.get();
	}

	void Fx::resize(u32 width, u32 height, u8 scale) {
		if (width * height == 0) {
			LogW("Incorrect dimensions.");
			return;
		}

		scale = std::min(scale, u8(4));
		m_width = width * scale;
		m_height = height * scale;
		m_scale = scale;

		SDL_SetWindowSize(m_window, m_width, m_height);

		m_screenBuffer->resize(width, height);
		m_graphics->setTarget(m_screenBuffer.get());

		SDL_DestroyTexture(m_backBuffer);
		m_backBuffer = SDL_CreateTexture(
				m_renderer,
				SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,
				width, height
		);

		m_graphics->unclip();
	}

	void Fx::setTitle(const Str& title) {
		SDL_SetWindowTitle(m_window, title.c_str());
	}

	Str Fx::title() const {
		return Str(SDL_GetWindowTitle(m_window));
	}

	Image* Fx::createImage(u32 width, u32 height) {
		m_imageResources.push_back(Ptr<Image>(new Image(width, height)));
		return m_imageResources.back().get();
	}

	void Fx::warpMouse(i32 x, i32 y) {
		SDL_WarpMouseInWindow(m_window, x * m_scale, y * m_scale);
	}

	void Fx::updateInput(SDL_Event& evt) {
		for (auto& i : m_buttons) {
			i.pressed = false;
			i.released = false;
		}
		for (auto& i : m_mouse) {
			i.pressed = false;
			i.released = false;
		}

		while (SDL_PollEvent(&evt)) {
			if (evt.type == SDL_QUIT) m_running = false;
			switch (evt.type) {
				default: break;
				case SDL_KEYDOWN: {
					i32 key = evt.key.keysym.sym;
					auto [hasKey, btn] = keyMapGetButton(u32(key));
					if (hasKey) {
						m_buttons[btn].pressed = true;
						m_buttons[btn].down = true;
					}
				} break;
				case SDL_KEYUP: {
					i32 key = evt.key.keysym.sym;
					auto [hasKey, btn] = keyMapGetButton(u32(key));
					if (hasKey) {
						m_buttons[btn].released = true;
						m_buttons[btn].down = false;
					}
				} break;
				case SDL_MOUSEBUTTONDOWN: {
					u32 btn = evt.button.button;
					m_mouse[btn-1].pressed = true;
					m_mouse[btn-1].down = true;
					m_mouseX = u32(evt.button.x / m_scale);
					m_mouseY = u32(evt.button.y / m_scale);
				} break;
				case SDL_MOUSEBUTTONUP: {
					u32 btn = evt.button.button;
					m_mouse[btn-1].released = true;
					m_mouse[btn-1].down = false;
				} break;
				case SDL_MOUSEMOTION: {
					m_mouseX = u32(evt.motion.x / m_scale);
					m_mouseY = u32(evt.motion.y / m_scale);
				} break;
			}
		}
	}

	Image* Fx::loadImage(const Str& fileName) {
		return loadImageD(fileName, DitherMode::None);
	}

	u8 Fx::closestPaletteColor(RGB col) {
		u8 closest = 0;
		u8 minR = 255;
		u8 minG = 255;
		u8 minB = 255;
		for (u8 i = 0; i < m_colorPalette.size(); i++) {
			RGB pcol = m_colorPalette[i];

			u8 dx = u8(std::abs(col.r - pcol.r));
			u8 dy = u8(std::abs(col.g - pcol.g));
			u8 dz = u8(std::abs(col.b - pcol.b));
			if ((dx + dy + dz) < (minR + minG + minB)) {
				closest = i;
				minR = dx;
				minG = dy;
				minB = dz;
			}
		}
		return closest;
	}

	Image* Fx::loadImageD(const Str& fileName, DitherMode ditherMode) {
		int w, h, comp;
		u8* data = stbi_load(fileName.c_str(), &w, &h, &comp, STBI_rgb_alpha);
		if (data) {
			Image* img = loadImageR(u32(w), u32(h), u32(comp), data, ditherMode);
			stbi_image_free(data);
			return img;
		}
		return nullptr;
	}

	u8 Fx::dither(i32 x, i32 y, u8 comp, DitherMode ditherMode) {
		int size = 0;
		const double* mat;
		switch (ditherMode) {
			default:
			case DitherMode::None: return comp;
			case DitherMode::Mode2x2: size = 2; mat = &DITHER_2x2[0]; break;
			case DitherMode::Mode3x3: size = 3; mat = &DITHER_3x3[0]; break;
			case DitherMode::Mode4x4: size = 4; mat = &DITHER_4x4[0]; break;
			case DitherMode::Mode8x8: size = 8; mat = &DITHER_8x8[0]; break;
		}

		return u8(std::max(std::min(comp + mat[(x % size) + (y % size) * size], 255.0), 0.0));
	}

	State Fx::getButtonState(const Str& name) const {
		static const Str NAMES[ButtonCount] = {
				"left",
				"right",
				"up",
				"down",
				"x",
				"y",
				"select"
		};

		auto beg = std::begin(NAMES);
		auto end = std::end(NAMES);
		auto it = std::find_if(beg, end,
				[&name](const std::string& s) {
					return std::equal(s.cbegin(), s.cend(), name.cbegin(), name.cend(),
							[](auto c1, auto c2) {
								return std::tolower(c2) == c1;
							}
					);
				}
		);

		if (it != end) {
			return m_buttons[it - beg];
		}

		return State();
	}

	State Fx::getMouseState(const Str& name) const {
		static const Str NAMES[ButtonCount] = {
				"left",
				"middle",
				"right",
				"x1",
				"x2"
		};

		auto beg = std::begin(NAMES);
		auto end = std::end(NAMES);
		auto it = std::find_if(beg, end,
				[&name](const std::string& s) {
					return std::equal(s.cbegin(), s.cend(), name.cbegin(), name.cend(),
							[](auto c1, auto c2) {
								return std::tolower(c2) == c1;
							}
					);
				}
		);

		if (it != end) {
			u8 pos = u8(it - beg);
			return m_mouse[pos];
		}

		return State();
	}

	Animator* Fx::createAnimator(u32 width, u32 height, u32 rows, u32 cols) {
		m_animators.push_back(std::make_unique<Animator>(width, height, rows, cols));
		return m_animators.back().get();
	}

	void Fx::updateAnimators(float dt) {
		for (auto&& anim : m_animators) {
			anim->update(dt);
		}

		/// 3D Animators
		for (auto&& model : m_models) {
			model->update(dt);
		}
	}

	Sound* Fx::createSound() {
		m_sounds.push_back(std::make_unique<Sound>("", "", "", "", 0.1f));
		return m_sounds.back().get();
	}

	void Fx::playSound(u32 channel, Sound* sound, bool loop) {
		m_audioSystem->playSingle(channel, sound, loop);
	}

	void Fx::playSoundAuto(Sound* sound, bool loop) {
		bool found = false;
		u32 chan = 0;
		for (u32 i = 0; i < AUS_CHANNEL_COUNT; i++) {
			if (!m_audioSystem->channels()[i]->playing()) {
				found = true;
				chan = i;
				break;
			}
		}

		if (found) {
			playSound(chan, sound, loop);
		} else { // Borrow
			m_audioSystem->stop(0);
			playSound(0, sound, loop);
		}
	}

	Image* Fx::createImageFromPixels(u32 width, u32 height, const byte* pixels) {
		Image* img = createImage(width, height);
		for (u32 i = 0; i < width * height; i++) {
			img->data()[i] = pixels[i];
		}
		return img;
	}

	Image* Fx::loadImageFromMemory(const byte* pixels, unsigned long size, DitherMode ditherMode) {
		int w, h, comp;
		u8* data = stbi_load_from_memory(pixels, int(size), &w, &h, &comp, STBI_rgb_alpha);
		if (data) {
			Image* img = loadImageR(u32(w), u32(h), u32(comp), data, ditherMode);
			stbi_image_free(data);
			return img;
		}
		return nullptr;
	}

	Image* Fx::loadImageR(u32 w, u32 h, u32 comp, const byte* pixels, DitherMode ditherMode) {
		Image* img = createImage(w, h);
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				int i = (x + y * w) * comp;
				RGB icol{ 0, 0, 0 };
				if (comp >= 1) {
					icol.r = pixels[i + 0];
				}
				if (comp >= 2) {
					icol.g = pixels[i + 1];
				}
				if (comp >= 3) {
					icol.b = pixels[i + 2];
				}
				if (comp == 4) {
					if (pixels[i + 3] <= 128) {
						img->set(x, y, 0xFF);
						continue;
					}
				}
				icol.r = dither(x, y, icol.r, ditherMode);
				icol.g = dither(x, y, icol.g, ditherMode);
				icol.b = dither(x, y, icol.b, ditherMode);
				img->set(x, y, closestPaletteColor(icol));
			}
		}
		return img;
	}

	// Split a String into a string array at a given token
	static void split(
		const Str& in,
		Vec<Str>& out,
		Str token
	) {
		out.clear();
		std::string temp;
		for (int i = 0; i < int(in.size()); i++) {
			std::string test = in.substr(i, token.size());
			if (test == token) 	{
				if (!temp.empty()) 	{
					out.push_back(temp);
					temp.clear();
					i += (int)token.size() - 1;
				} else {
					out.push_back("");
				}
			} else if (i + token.size() >= in.size()) {
				temp += in.substr(i, token.size());
				out.push_back(temp);
				break;
			} else {
				temp += in[i];
			}
		}
	}

	static void calculateNormals(Vec<ModelVertex>& verts, Vec<u32> inds) {
		for (u32 i = 0; i < inds.size(); i+=3) {
			ModelVertex& v0 = verts[inds[i + 0]];
			ModelVertex& v1 = verts[inds[i + 1]];
			ModelVertex& v2 = verts[inds[i + 2]];

			glm::vec3 p0 = glm::vec3(v0.x, v0.y, v0.z);
			glm::vec3 p1 = glm::vec3(v1.x, v1.y, v1.z);
			glm::vec3 p2 = glm::vec3(v2.x, v2.y, v2.z);

			glm::vec3 e0 = p1 - p0;
			glm::vec3 e1 = p2 - p0;
			glm::vec3 n = glm::cross(e0, e1);

			v0.nx += n.x; v0.ny += n.y; v0.nz += n.z;
			v1.nx += n.x; v1.ny += n.y; v1.nz += n.z;
			v2.nx += n.x; v2.ny += n.y; v2.nz += n.z;
		}

		for (ModelVertex& v0 : verts) {
			glm::vec3 n = glm::vec3(v0.nx, v0.ny, v0.nz);
			n = glm::normalize(n);
			v0.nx = n.x; v0.ny = n.y; v0.nz = n.z;
		}
	}

	Model* Fx::loadModel(const Str& fileName) {
		std::ifstream fp(fileName, std::ios::binary | std::ios::in);
		if (fp.good()) {
			// Initial file data
			Vec<u32> keyFrames;
			Map<u32, Vec<glm::vec3>> vertices;
			Vec<glm::vec2> uvs;
			Vec<Face> indices;

			// Read file
			size_t len = 0;
			fp.seekg(0, std::ios::end);
			len = fp.tellg();
			fp.seekg(0, std::ios::beg);

			u8* data = new u8[len];
			fp.read((char*) data, len);
			fp.close();

			// Read TMDHeader
			ByteReader br(data, len);
			TMDHeader header;
			header.identifier[0] = br.nextValue<u8>();
			header.identifier[1] = br.nextValue<u8>();
			header.identifier[2] = br.nextValue<u8>();

			assert(header.identifier[0] == 'T' &&
					header.identifier[1] == 'M' &&
					header.identifier[2] == 'D' &&
					"Invalid 3D model format.");

			header.vertexCount = br.nextValue<u32>();
			header.uvCount = br.nextValue<u32>();
			header.faceCount = br.nextValue<u32>();
			header.keyframeCount = br.nextValue<u32>();
			header.hasUVs = br.nextValue<u8>();

			// Read keyframe data
			for (u32 k = 0; k < header.keyframeCount; k++) {
				keyFrames.push_back(br.nextValue<u32>());
			}

			// Read vertex positions
			for (u32 k : keyFrames) {
				for (u32 v = 0; v < header.vertexCount; v++) {
					float x = br.nextValue<float>();
					float y = br.nextValue<float>();
					float z = br.nextValue<float>();
					vertices[k].push_back(glm::vec3(x, y, z));
				}
			}

			// Read UVs
			for (u32 i = 0; i < header.uvCount; i++) {
				float x = br.nextValue<float>();
				float y = br.nextValue<float>();
				uvs.push_back(glm::vec2(x, y));
			}

			// Read Faces
			for (u32 i = 0; i < header.faceCount; i++) {
				u32 vi = br.nextValue<u32>();
				u32 ti = 0;
				if (header.hasUVs > 0) {
					ti = br.nextValue<u32>();
				}
				indices.emplace_back(vi, ti);
			}

			delete[] data;

			// Process data
			std::sort(keyFrames.begin(), keyFrames.end());

			Map<u32, u32> resultIndexMap;
			Map<u32, Vec<ModelVertex>> finalVertices;
			Vec<u32> finalIndices;

			// Use the first keyframe to calculate the index array
			for (u32 i = 0; i < indices.size(); i++) {
				u32 k = keyFrames[0];
				Face ci = indices[i];
				glm::vec3 vpos = vertices[k][ci.vi];
				glm::vec2 vuv(0.0f);

				if (!uvs.empty()) {
					vuv = uvs[ci.ti];
				}

				if (finalVertices[k].empty()) {
					finalVertices[k].resize(vertices[k].size());
				}

				u32 mvi = ci.vi;
				finalVertices[k][mvi].x = vpos.x;
				finalVertices[k][mvi].y = vpos.y;
				finalVertices[k][mvi].z = vpos.z;
				finalVertices[k][mvi].s = vuv.x;
				finalVertices[k][mvi].t = vuv.y;
				finalVertices[k][mvi].nx = 0;
				finalVertices[k][mvi].ny = 0;
				finalVertices[k][mvi].nz = 0;

				finalIndices.push_back(u32(mvi));
			}

			for (u32 i = 1; i < keyFrames.size(); i++) {
				u32 k = keyFrames[i];
				for (u32 j = 0; j < indices.size(); j++) {
					Face ci = indices[j];
					glm::vec3 vpos = vertices[k][ci.vi];
					glm::vec2 vuv(0.0f);

					if (!uvs.empty()) {
						vuv = uvs[ci.ti];
					}

					if (finalVertices[k].empty()) {
						finalVertices[k].resize(vertices[k].size());
					}

					u32 mvi = ci.vi;
					finalVertices[k][mvi].x = vpos.x;
					finalVertices[k][mvi].y = vpos.y;
					finalVertices[k][mvi].z = vpos.z;
					finalVertices[k][mvi].s = vuv.x;
					finalVertices[k][mvi].t = vuv.y;
					finalVertices[k][mvi].nx = 0;
					finalVertices[k][mvi].ny = 0;
					finalVertices[k][mvi].nz = 0;
				}
			}

			// Generate normals
			for (u32 k : keyFrames) {
				Vec<ModelVertex>& verts = finalVertices[k];
				calculateNormals(verts, finalIndices);
			}

			m_models.push_back(Ptr<Model>(new Model()));
			Model* model = m_models.back().get();
			model->m_vertices = finalVertices;
			model->m_indices = finalIndices;
			model->m_frameCount = keyFrames.size();
			model->m_vertexCount = finalVertices[keyFrames[0]].size();
			return model;
		}
		return nullptr;
	}

}

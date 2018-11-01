//
// Created by Twister on 26/09/2018.
//

#ifndef TGF_FX_H
#define TGF_FX_H

#include "Graphics.h"
#include "Animator.h"
#include "AudioSystem.h"
#include "Font.h"
#include "Model.h"

#include "SDL2/SDL.h"

#define TGF_TIMESTEP (1.0/60.0)

namespace tgf {
	class Game {
	public:
		virtual void create() = 0;
		virtual void update(float dt) = 0;
		virtual void draw(Graphics& graphics) = 0;
	};

	struct State {
		bool pressed, released, down;
		State() : pressed(false), released(false), down(false) {}
	};

	enum DitherMode {
		None = 0,
		Mode2x2 = 2,
		Mode3x3 = 3,
		Mode4x4 = 4,
		Mode8x8 = 8
	};

	enum Button {
		ButtonLeft = 0,
		ButtonRight,
		ButtonUp,
		ButtonDown,
		ButtonX,
		ButtonY,
		ButtonSelect,
		ButtonCount
	};

	class Fx {
		friend class Lua;
	public:
		virtual ~Fx();

		void init(const Str& title, u32 width, u32 height, u8 scale);
		u8 color(const Str& name);
		void flip();
		void quit() { m_running = false; }
		int run(Game* game);
		void resize(u32 width, u32 height, u8 scale);

		Image* createImage(u32 width, u32 height);
		Image* createImageFromPixels(u32 width, u32 height, const byte* pixels);
		Image* loadImage(const Str& fileName);
		Image* loadImageFromMemory(const byte* pixels, unsigned long size, DitherMode ditherMode);
		Animator* createAnimator(u32 width, u32 height, u32 rows, u32 cols);
		Sound* createSound();
		Model* loadModel(const Str& fileName);

		void playSound(u32 channel, Sound* sound, bool loop);
		void playSoundAuto(Sound* sound, bool loop);

		bool buttonPressed(const Str& name) const { return getButtonState(name).pressed; }
		bool buttonReleased(const Str& name) const { return getButtonState(name).released; }
		bool buttonDown(const Str& name) const { return getButtonState(name).down; }
		bool mousePressed(const Str& name) const { return getMouseState(name).pressed; }
		bool mouseReleased(const Str& name) const { return getMouseState(name).released; }
		bool mouseDown(const Str& name) const { return getMouseState(name).down; }

		i32 mouseX() const { return m_mouseX; }
		i32 mouseY() const { return m_mouseY; }
		void warpMouse(i32 x, i32 y);

		Str title() const;
		void setTitle(const Str& title);
		u32 width() const { return m_screenBuffer->width(); }
		u32 height() const { return m_screenBuffer->height(); }
		Array<RGB, 16>& palette() { return m_colorPalette; }
		Graphics* graphics() { return m_graphics.get(); }
		Font defaultFont() { return *m_defaultFont.get(); }

		u8 closestPaletteColor(RGB col);
		u8 dither(i32 x, i32 y, u8 comp, DitherMode ditherMode);

		static Fx& instance();
	protected:
		Fx() = default;

		State getButtonState(const Str& name) const;
		State getMouseState(const Str& name) const;

		void updateInput(SDL_Event& evt);
		void updateAnimators(float dt);

		Image* loadImageD(const Str& fileName, DitherMode ditherMode);
		Image* loadImageR(u32 w, u32 h, u32 comp, const byte* pixels, DitherMode ditherMode);

	private:
		static Ptr<Fx> s_instance;

		Array<RGB, 16> m_colorPalette;
		Ptr<Graphics> m_graphics;
		Ptr<Image> m_screenBuffer;
		Ptr<Game> m_game;
		Ptr<Font> m_defaultFont;

		bool m_running;
		u32 m_width, m_height;
		u8 m_scale;

		/// Input
		Array<State, ButtonCount> m_buttons;
		Array<State, 6> m_mouse;
		u32 m_mouseX, m_mouseY;

		/// Audio
		Ptr<AudioSystem> m_audioSystem;

		/// Resources
		Vec<Ptr<Image>> m_imageResources;
		Vec<Ptr<Animator>> m_animators;
		Vec<Ptr<Sound>> m_sounds;
		Vec<Ptr<Model>> m_models;

		/// SDL2
		SDL_Window* m_window;
		SDL_Renderer* m_renderer;
		SDL_Texture* m_backBuffer;
	};

}


#endif //TGF_FX_H

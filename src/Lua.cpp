//
// Created by Twister on 26/09/2018.
//

#include "Lua.h"

#include "kaguya/kaguya.hpp"

#include <optional>
#include "Fx.h"
#include "Log.h"
#include "Vector.h"
#include "Font.h"
#include "AudioSystem.h"
#include "3DRendering.h"
#include "Model.h"

#define GETTER(T, v) [](const T* self) { return self->v; }
#define SETTER(T, S, v) [](T* self, S nv) { self->v = nv; }

namespace tgf {
	static float radians(float a) {
		return glm::radians(a);
	}

	static float degrees(float a) {
		return glm::degrees(a);
	}

	Ptr<Lua> Lua::s_instance;
	Lua& Lua::instance() {
		if (!s_instance) {
			s_instance = Ptr<Lua>(new Lua());
		}
		return *s_instance.get();
	}

	void Lua::init() {
		m_L = Ptr<kaguya::State>(new kaguya::State());
		kaguya::State& L = *m_L.get();

		L.openlibs();
		L.setErrorHandler([](int status, const char* msg) {
			LogE("[Lua(err #", status, ") ", msg);
		});

		L["Framework"].setClass(kaguya::UserdataMetatable<Fx>()
				.addFunction("color", &Fx::color)
				.addFunction("flip", &Fx::flip)
				.addFunction("quit", &Fx::quit)
				.addFunction("resize", &Fx::resize)
				.addFunction("buttonPressed", &Fx::buttonPressed)
				.addFunction("buttonReleased", &Fx::buttonReleased)
				.addFunction("buttonDown", &Fx::buttonDown)
				.addFunction("mousePressed", &Fx::mousePressed)
				.addFunction("mouseReleased", &Fx::mouseReleased)
				.addFunction("mouseDown", &Fx::mouseDown)
				.addFunction("warpMouse", &Fx::warpMouse)
				.addFunction("createAnimator", &Fx::createAnimator)
				.addFunction("createSound", &Fx::createSound)
				.addFunction("loadModel", &Fx::loadModel)
				.addOverloadedFunctions("title", &Fx::title, &Fx::setTitle)
				.addOverloadedFunctions("createImage", &Fx::createImage, &Fx::createImageFromPixels)
				.addOverloadedFunctions("playSound", &Fx::playSound, &Fx::playSoundAuto)
				.addOverloadedFunctions("loadImage", &Fx::loadImage, &Fx::loadImageD, &Fx::loadImageFromMemory)
				.addProperty("mouseX", &Fx::mouseX)
				.addProperty("mouseY", &Fx::mouseY)
				.addProperty("width", &Fx::width)
				.addProperty("height", &Fx::height)
				.addProperty("graphics", &Fx::graphics)
				.addProperty("defaultFont", &Fx::defaultFont)
		);

		L["Graphics"].setClass(kaguya::UserdataMetatable<Graphics>()
				.setConstructors<Graphics(Image*)>()
				.addFunction("clear", &Graphics::clear)
				.addFunction("pixel", &Graphics::pixel)
				.addFunction("line", &Graphics::line)
				.addFunction("rect", &Graphics::rect)
				.addFunction("circle", &Graphics::circle)
				.addFunction("tile", &Graphics::tile)
				.addFunction("text", &Graphics::text)
				.addFunction("color", &Graphics::color)
				.addFunction("vertex", &Graphics::vertex)
				.addFunction("end3D", &Graphics::end)
				.addFunction("bind", &Graphics::bind)
				.addFunction("model", &Graphics::model)
				.addFunction("smooth", &Graphics::smooth)
				.addFunction("lighting", &Graphics::lighting)
				.addFunction("lightDirection", &Graphics::lightDirection)
				.addFunction("matrixMode", &Graphics::matrixMode)
				.addFunction("pushMatrix", &Graphics::pushMatrix)
				.addFunction("popMatrix", &Graphics::popMatrix)
				.addFunction("identity", &Graphics::identity)
				.addFunction("translate", &Graphics::translate)
				.addFunction("rotate", &Graphics::rotate)
				.addFunction("scale", &Graphics::scale)
				.addFunction("perspective", &Graphics::perspective)
				.addFunction("ortho", &Graphics::ortho)
				.addOverloadedFunctions("begin3D", &Graphics::begin, &Graphics::beginMode)
				.addOverloadedFunctions("remap", &Graphics::remap, &Graphics::remapReset, &Graphics::remapNonTransparent)
				.addOverloadedFunctions("sprite", &Graphics::sprite, &Graphics::spriteAll, &Graphics::spriteFX, &Graphics::spriteNF)
				.addOverloadedFunctions("clip", &Graphics::clip, &Graphics::unclip)
				.addOverloadedFunctions("transparency", &Graphics::transparency, &Graphics::resetTransparency)
				.addOverloadedFunctions("target", &Graphics::target, &Graphics::setTarget)
		);

		L["Image"].setClass(kaguya::UserdataMetatable<Image>()
				.addProperty("width", &Image::width)
				.addProperty("height", &Image::height)
				.addProperty("data", &Image::data)
				.addFunction("get", &Image::get)
				.addFunction("set", &Image::set)
		);

		L["Vertex"].setClass(kaguya::UserdataMetatable<ModelVertex>()
				.addPropertyAny("x", GETTER(ModelVertex, x), SETTER(ModelVertex, float, x))
				.addPropertyAny("y", GETTER(ModelVertex, y), SETTER(ModelVertex, float, y))
				.addPropertyAny("z", GETTER(ModelVertex, z), SETTER(ModelVertex, float, z))
				.addPropertyAny("nx", GETTER(ModelVertex, nx), SETTER(ModelVertex, float, nx))
				.addPropertyAny("ny", GETTER(ModelVertex, ny), SETTER(ModelVertex, float, ny))
				.addPropertyAny("nz", GETTER(ModelVertex, nz), SETTER(ModelVertex, float, nz))
				.addPropertyAny("s", GETTER(ModelVertex, s), SETTER(ModelVertex, float, s))
				.addPropertyAny("t", GETTER(ModelVertex, t), SETTER(ModelVertex, float, t))
		);

		L["Model"].setClass(kaguya::UserdataMetatable<Model>()
				.addFunction("getVertex", &Model::getVertexLua)
				.addFunction("getIndex", &Model::getIndexLua)
				.addFunction("addAnimation", &Model::addAnimation)
				.addFunction("play", &Model::play)
				.addProperty("vertexCount", &Model::vertexCount)
				.addProperty("indexCount", &Model::indexCount)
		);

		L["Vector"].setClass(kaguya::UserdataMetatable<Vector>()
				.addFunction("dot", &Vector::dot)
				.addFunction("normalized", &Vector::normalized)
				.addFunction("perp", &Vector::perp)
				.addFunction("set", &Vector::set)
				.addFunction("__len", &Vector::length)
				.addFunction("__add", &Vector::add)
				.addFunction("__sub", &Vector::sub)
				.addFunction("__unm", &Vector::neg)
				.addFunction("__div", &Vector::div)
				.addFunction("__tostring", &Vector::toString)
				.addOverloadedFunctions("__mul", &Vector::mul, &Vector::mulF)
				.addProperty("angle", &Vector::angle)
				.addProperty("length", &Vector::length)
				.addPropertyAny("x", GETTER(Vector, x), SETTER(Vector, float, x))
				.addPropertyAny("y", GETTER(Vector, y), SETTER(Vector, float, y))
		);

		L["Rect"].setClass(kaguya::UserdataMetatable<Rect>()
				.setConstructors<Rect(), Rect(i32, i32, i32, i32)>()
				.addPropertyAny("x", GETTER(Rect, x), SETTER(Rect, i32, x))
				.addPropertyAny("y", GETTER(Rect, y), SETTER(Rect, i32, y))
				.addPropertyAny("w", GETTER(Rect, w), SETTER(Rect, i32, w))
				.addPropertyAny("h", GETTER(Rect, h), SETTER(Rect, i32, h))
		);

		L["Font"].setClass(kaguya::UserdataMetatable<Font>()
				.setConstructors<Font(Image*, const Str&, u32, u32)>()
				.addFunction("stringWidth", &Font::stringWidth)
				.addProperty("spacing", &Font::spacing, &Font::setSpacing)
				.addProperty("charMap", &Font::charMap, &Font::setCharMap)
				.addProperty("image", &Font::image)
				.addProperty("columns", &Font::columns)
				.addProperty("rows", &Font::rows)
		);

		L["Animator"].setClass(kaguya::UserdataMetatable<Animator>()
				.addFunction("add", &Animator::add)
				.addFunction("play", &Animator::play)
				.addFunction("reset", &Animator::reset)
				.addProperty("clip", &Animator::clip)
				.addProperty("animation", &Animator::animation)
		);

		L["Sound"].setClass(kaguya::UserdataMetatable<Sound>()
				.addFunction("setNotes", &Sound::setNotes)
				.addFunction("setVolumes", &Sound::setVolumes)
				.addFunction("setEffects", &Sound::setEffects)
				.addFunction("setWaveForms", &Sound::setWaveForms)
				.addFunction("set", &Sound::set)
				.addProperty("speed", &Sound::speed, &Sound::setSpeed)
		);

		L["math"]["radians"] = &radians;
		L["math"]["degrees"] = &degrees;

		L["Fx"] = &Fx::instance();
	}

	Lua::~Lua() = default;

	void Lua::runScript(const Str& code) {
		kaguya::State& L = *m_L.get();
		L.dostring(code);
	}

	void Lua::onCreate() {
		kaguya::State& L = *m_L.get();
		kaguya::LuaFunction fun = L["create"];
		if (fun.type() == LUA_TFUNCTION) {
			fun.call<void>();
		}
	}

	void Lua::onUpdate(float dt) {
		kaguya::State& L = *m_L.get();
		kaguya::LuaFunction fun = L["update"];
		if (fun.type() == LUA_TFUNCTION) {
			fun.call<void>(dt);
		}
	}

	void Lua::onDraw(Graphics* g) {
		kaguya::State& L = *m_L.get();
		kaguya::LuaFunction fun = L["draw"];
		if (fun.type() == LUA_TFUNCTION) {
			fun.call<void>(g);
		}
	}

}

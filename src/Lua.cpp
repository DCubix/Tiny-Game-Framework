//
// Created by Twister on 26/09/2018.
//

#include "Lua.h"

#include <optional>
#include "Fx.h"
#include "Log.h"
#include "Vector.h"
#include "Font.h"
#include "AudioSystem.h"
#include "3DRendering.h"
#include "Model.h"

#define SOL_CHECK_ARGUMENTS 0
#define SOL_NO_CHECK_NUMBER_PRECISION 1
#include "sol.hpp"

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
		L = luaL_newstate();
		luaL_openlibs(L);

		sol::state_view vL(L);

		vL.new_simple_usertype<Fx>("Framework",
				"color", &Fx::color,
				"flip", &Fx::flip,
				"quit", &Fx::quit,
				"resize", &Fx::resize,
				"buttonPressed", &Fx::buttonPressed,
				"buttonReleased", &Fx::buttonReleased,
				"buttonDown", &Fx::buttonDown,
				"mousePressed", &Fx::mousePressed,
				"mouseReleased", &Fx::mouseReleased,
				"mouseDown", &Fx::mouseDown,
				"warpMouse", &Fx::warpMouse,
				"createSound", &Fx::createSound,
				"loadModel", &Fx::loadModel,
				"title", sol::overload(&Fx::title, &Fx::setTitle),
				"createImage", sol::overload(&Fx::createImage, &Fx::createImageFromPixels),
				"playSound", sol::overload(&Fx::playSound, &Fx::playSoundAuto),
				"loadImage", sol::overload(&Fx::loadImage, &Fx::loadImageD),
				"mouseX", sol::readonly_property(&Fx::mouseX),
				"mouseY", sol::readonly_property(&Fx::mouseY),
				"width", sol::readonly_property(&Fx::width),
				"height", sol::readonly_property(&Fx::height),
				"graphics", sol::readonly_property(&Fx::graphics),
				"defaultFont", sol::readonly_property(&Fx::defaultFont)
		);

		vL.new_simple_usertype<Graphics>("Graphics",
				sol::constructors<Graphics(Image*)>(),
				"clear", &Graphics::clear,
				"pixel", &Graphics::pixel,
				"line", &Graphics::line,
				"rect", &Graphics::rect,
				"circle", &Graphics::circle,
				"tile", &Graphics::tile,
				"text", &Graphics::text,
				"color", &Graphics::color,
				"vertex", &Graphics::vertex,
				"end3D", &Graphics::end,
				"bind", &Graphics::bind,
				"model", &Graphics::model,
				"smooth", &Graphics::smooth,
				"light", sol::overload(&Graphics::lightDirection, &Graphics::lighting),
				"matrixMode", &Graphics::matrixMode,
				"pushMatrix", &Graphics::pushMatrix,
				"popMatrix", &Graphics::popMatrix,
				"identity", &Graphics::identity,
				"translate", &Graphics::translate,
				"rotate", &Graphics::rotate,
				"scale", &Graphics::scale,
				"perspective", &Graphics::perspective,
				"ortho", &Graphics::ortho,
				"begin3D", sol::overload(&Graphics::begin, &Graphics::beginMode),
				"remap", sol::overload(&Graphics::remap, &Graphics::remapReset, &Graphics::remapNonTransparent),
				"sprite", sol::overload(&Graphics::sprite, &Graphics::spriteAll, &Graphics::spriteFX, &Graphics::spriteNF),
				"clip", sol::overload(&Graphics::clip, &Graphics::unclip),
				"key", sol::overload(&Graphics::transparency, &Graphics::resetTransparency),
				"target", sol::property(&Graphics::targetLua, &Graphics::setTarget)
		);

		vL.new_simple_usertype<Image>("Image",
				"width", sol::readonly_property(&Image::width),
				"height", sol::readonly_property(&Image::height),
				"data", sol::readonly_property(&Image::data),
				"get", &Image::get,
				"set", &Image::set,
				/* Animation */
				"animation", sol::readonly_property(&Image::animation),
				"add", &Image::addL,
				"play", &Image::play,
				"reset", &Image::reset,
				"animate", &Image::animate
		);

		vL.new_simple_usertype<ModelVertex>("Vertex",
				"x", &ModelVertex::x,
				"y", &ModelVertex::y,
				"z", &ModelVertex::z,
				"nx", &ModelVertex::nx,
				"ny", &ModelVertex::ny,
				"nz", &ModelVertex::nz,
				"s", &ModelVertex::s,
				"t", &ModelVertex::t
		);

		vL.new_simple_usertype<Model>("Model",
				"getVertex", &Model::getVertexLua,
				"getIndex", &Model::getIndexLua,
				"addAnimation", &Model::addAnimation,
				"play", &Model::play,
				"vertexCount", sol::readonly_property(&Model::vertexCount),
				"indexCount", sol::readonly_property(&Model::indexCount)
		);

		vL.new_simple_usertype<Vector>("Vector",
				sol::constructors<Vector(), Vector(float), Vector(float, float)>(),
				"dot", &Vector::dot,
				"normalized", &Vector::normalized,
				"perp", &Vector::perp,
				"set", &Vector::set,
				"__len", &Vector::length,
				"__add", &Vector::add,
				"__sub", &Vector::sub,
				"__unm", &Vector::neg,
				"__div", &Vector::div,
				"__tostring", &Vector::toString,
				"__mul", sol::overload(&Vector::mul, &Vector::mulF),
				"angle", sol::readonly_property(&Vector::angle),
				"length", sol::readonly_property(&Vector::length),
				"x", &Vector::x,
				"y", &Vector::y
		);

		vL.new_simple_usertype<Rect>("Rect",
				sol::constructors<Rect(), Rect(i32, i32, i32, i32)>(),
				"x", &Rect::x,
				"y", &Rect::y,
				"w", &Rect::w,
				"h", &Rect::h
		);

		vL.new_simple_usertype<Font>("Font",
				sol::constructors<Font(Image*, const Str&, u32, u32)>(),
				"stringWidth", &Font::stringWidth,
				"spacing", sol::property(&Font::spacing, &Font::setSpacing),
				"charMap", sol::property(&Font::charMap, &Font::setCharMap),
				"image", sol::readonly_property(&Font::image),
				"columns", sol::readonly_property(&Font::columns),
				"rows", sol::readonly_property(&Font::rows),
				"height", sol::readonly_property(&Font::height)
		);

		vL.new_simple_usertype<Sound>("Sound",
				"setNotes", &Sound::setNotes,
				"setVolumes", &Sound::setVolumes,
				"setEffects", &Sound::setEffects,
				"setWaveForms", &Sound::setWaveForms,
				"set", &Sound::set,
				"speed", sol::property(&Sound::speed, &Sound::setSpeed)
		);

		vL["math"]["radians"] = &radians;
		vL["math"]["degrees"] = &degrees;

		vL["Fx"] = &Fx::instance();
	}

	Lua::~Lua() = default;

	void Lua::runScript(const Str& code) {
		sol::state_view vL(L);
		try {
			vL.script(code);
		} catch (const sol::error& err) {
			LogE("[Lua Error] ", err.what());
		}
	}

	void Lua::onCreate() {
		sol::state_view vL(L);
		auto&& fun = vL["create"];
		if (fun.get_type() == sol::type::function) {
			fun.call();
		}
	}

	void Lua::onUpdate(float dt) {
		sol::state_view vL(L);
		auto&& fun = vL["update"];
		if (fun.get_type() == sol::type::function) {
			fun.call(dt);
		}
	}

	void Lua::onDraw(Graphics* g) {
		sol::state_view vL(L);
		auto&& fun = vL["draw"];
		if (fun.get_type() == sol::type::function) {
			fun.call(g);
		}
	}

}

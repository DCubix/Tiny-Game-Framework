//
// Created by Twister on 26/09/2018.
//

#ifndef TGF_LUA_H
#define TGF_LUA_H

#include <lua.hpp>

#include "Types.h"
#include "Graphics.h"

namespace kaguya {
	class State;
}

namespace tgf {

	class Lua {
	public:
		virtual ~Lua();

		void init();

		void runScript(const Str& fileName);

		void onCreate();
		void onUpdate(float dt);
		void onDraw(Graphics* g);

		static Lua& instance();

	protected:
		Lua() = default;

	private:
		static Ptr<Lua> s_instance;

		Ptr<kaguya::State> m_L;
	};
}


#endif //TGF_LUA_H

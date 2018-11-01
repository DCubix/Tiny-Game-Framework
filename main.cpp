#include <utility>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <streambuf>

#include "src/Fx.h"
#include "src/Lua.h"
using namespace tgf;

class Main : public Game {
public:
	explicit Main(Str code) : code(std::move(code)) { }
	virtual ~Main() = default;

	void create() final {
		Lua::instance().init();
		Lua::instance().runScript(code);
		Lua::instance().onCreate();
	}

	void update(float dt) final {
		Lua::instance().onUpdate(dt);
	}

	void draw(Graphics& g) final {
		Lua::instance().onDraw(&g);
	}

	Str code;
};

bool endsWith(Str mainStr, Str toMatch) {
	auto it = toMatch.begin();
	return mainStr.size() >= toMatch.size() &&
			std::all_of(std::next(mainStr.begin(),mainStr.size() - toMatch.size()), mainStr.end(), [&it](const char & c){
				return ::tolower(c) == ::tolower(*(it++))  ;
			} );
}

int main(int argc, const char** argv) {
	Str code = "print(\"No game loaded.\")\nFx:quit()";

#if 0
	std::ifstream t("threedee.lua");
	if (t.good()) {
		code = Str((std::istreambuf_iterator<char>(t)),
				std::istreambuf_iterator<char>());
		t.close();
	}
#else
	if (argc == 2) {
		const Str ARG1 = Str(argv[1]);
		if (endsWith(ARG1, "lua")) { // Running a plain Lua script (.lua)
			std::ifstream t(ARG1);
			if (t.good()) {
				code = Str((std::istreambuf_iterator<char>(t)),
						std::istreambuf_iterator<char>());
				t.close();
			} else {
				LogE("Invalid script file.");
				return 2;
			}
		} else {
			LogE("Invalid script file.");
			return 2;
		}
	}
#endif

	Fx& fx = Fx::instance();
	fx.init("Tiny Game Framework", 320, 240, 1);
	return fx.run(new Main(code));
}

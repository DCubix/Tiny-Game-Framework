#include <utility>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <streambuf>

#include "src/Fx.h"
#include "src/Lua.h"
using namespace tgf;

#ifdef WINDOWS
#include <direct.h>
#define ChangeDir _chdir
#else
#include "unistd.h"
#define ChangeDir chdir
#endif

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
			// Change working directory to the one the script file is in
			Str path = ARG1;
			Str fileName = path.substr(path.find_last_of('/')+1);
			std::replace(path.begin(), path.end(), '\\', '/');
			const Str dir = path.substr(0, path.find_last_of('/'));

			if (dir != fileName) {
				ChangeDir(dir.c_str());
			}

			std::ifstream t(fileName);
			if (t.good()) {
				code = Str((std::istreambuf_iterator<char>(t)),
						std::istreambuf_iterator<char>());
				t.close();
			} else {
				LogE("Invalid script file: File not found.");
				return 2;
			}
		} else {
			LogE("Invalid script file: Unknown format.");
			return 3;
		}
	}
#endif

	Fx& fx = Fx::instance();
	fx.init("Tiny Game Framework", 320, 240, 1);
	return fx.run(new Main(code));
}

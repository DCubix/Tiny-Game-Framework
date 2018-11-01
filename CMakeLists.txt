cmake_minimum_required(VERSION 3.11)
project(tgf)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL2 REQUIRED)
find_package(Lua REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS} ${LUA_INCLUDE_DIR} "src/glm")

if(WIN32 OR MINGW OR MSVC)
    add_definitions(-DSDL_MAIN_HANDLED)
    add_definitions(-DWINDOWS)
endif()

if(GNU OR MINGW)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj")
	message(STATUS "Using BIG OBJ")
endif()

file(GLOB_RECURSE KAG_SRC
	"src/kaguya/*.hpp"
)
file(GLOB_RECURSE GLM_SRC "src/glm/*.hpp")
file(GLOB SRC
	"src/*.h"
	"src/*.c"
	"src/*.cpp"
	"src/stb/*.h"
	"src/stb/*.cpp"
	"src/termcolor/*.hpp"
)

set(SRC ${SRC} ${GLM_SRC})
set(SRC ${SRC} ${KAG_SRC})

add_executable(tgf
        main.cpp
		${SRC}
)
target_link_libraries(tgf
        ${SDL2_LIBRARIES}
        ${LUA_LIBRARIES}
)
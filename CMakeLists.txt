# include SDL2 and SDL2_image
list(APPEND CMAKE_PREFIX_PATH "C:/dev/SDL2_image-2.6.3")
list(APPEND CMAKE_PREFIX_PATH "C:/dev/SDL2-2.28.3")

cmake_minimum_required(VERSION 3.8)

project(ZumaClone
	VERSION 1.0
	DESCRIPTION "A Zuma Clone"
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)

# generated executable
add_executable(${PROJECT_NAME} main.cpp)

# disable debug console window if running in Release
if (${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS /SUBSYSTEM:WINDOWS)
else()
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS /SUBSYSTEM:CONSOLE)
endif()

# source files for engine and game subdirectories

target_sources(${PROJECT_NAME} PRIVATE 
	engine/Animation.cpp
	engine/Animation.h
	engine/AssetManager.cpp
	engine/AssetManager.h
	engine/basics.cpp
	engine/basics.h
	engine/common.cpp
	engine/common.h
	engine/Engine.cpp
	engine/Engine.h
	engine/EntityManager.h
	engine/Sprite.cpp
	engine/Sprite.h
	engine/Texture.cpp
	engine/Texture.h
	engine/UI.cpp
	engine/UI.h
	game/Balls.cpp
	game/Balls.h
	game/Player.cpp
	game/Player.h
)

# header files directories for the given executable
# target_include_directories(${PROJECT_NAME} PRIVATE engine game)

# include SDL2 and SDL2_image

find_package(SDL2 REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)

find_package(SDL2_image REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_IMAGE_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2_image::SDL2_image)

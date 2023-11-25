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

# source files for engine and game subdirectories

target_sources(${PROJECT_NAME} PRIVATE 
	engine/Animation.cpp
	engine/AssetManager.cpp
	engine/basics.cpp
	engine/common.cpp
	engine/Engine.cpp
	engine/EntityManager.cpp
	engine/Sprite.cpp
	engine/Texture.cpp
	engine/UI.cpp
	game/Balls.cpp
	game/Player.cpp
)

# header files directories for the given executable
target_include_directories(${PROJECT_NAME} PRIVATE engine game)

# include SDL2 and SDL2_image

find_package(SDL2 REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)

find_package(SDL2_image REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_IMAGE_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2_image::SDL2_image)

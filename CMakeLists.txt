cmake_minimum_required(VERSION 3.8)

project(CatinkAdventures
	VERSION 1.0
	DESCRIPTION "A Zuma Clone"
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
list(
	APPEND 
	CMAKE_PREFIX_PATH 
	${CMAKE_SOURCE_DIR}/extern/SDL2-2.28.3 
	${CMAKE_SOURCE_DIR}/extern/SDL2_image-2.6.3 
	${CMAKE_SOURCE_DIR}/extern/SDL2_mixer-2.6.3
	${CMAKE_SOURCE_DIR}/extern/SDL2_ttf-2.20.2
)

set(GUI_TYPE "")

# disable debug console window if running in Release
if (${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
	if (APPLE)
		set(GUI_TYPE MACOSX_BUNDLE)
	elseif (WIN32)
		set(GUI_TYPE WIN32)
	endif()
endif()

# generated executable
add_executable(${PROJECT_NAME} ${GUI_TYPE} main.cpp)

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
	game/LevelData.h
	engine/SoundManager.h
	engine/Fade.h
	engine/Audio.h

	engine/UIElements/Text.h
	engine/UIElements/Button.h
	engine/UIElements/FlexContainer.h
)

# include SDL2

find_package(SDL2 REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)

# include SDL2_image

find_package(SDL2_image REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_IMAGE_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2_image::SDL2_image)

# include SDL2_ttf

find_package(SDL2_ttf REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_TTF_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2_ttf::SDL2_ttf)

# include SDL2_mixer

find_package(SDL2_mixer REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_MIXER_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2_mixer::SDL2_mixer)

cmake_minimum_required(VERSION 3.14)

project(CatinkAdventures
	VERSION 1.0
	DESCRIPTION "A Zuma Clone"
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)

include(./cmake/CPM.cmake)

CPMAddPackage(
	NAME SDL2
	GITHUB_REPOSITORY libsdl-org/SDL
	GIT_TAG release-2.28.5
	OPTIONS
		"SDL_TEST OFF"
		"SDL_SHARED ON"
		"SDL_STATIC OFF"
		"SDL2_DISABLE_INSTALL ON"
)

CPMAddPackage(
	NAME SDL2_image
	GITHUB_REPOSITORY libsdl-org/SDL_image
	GIT_TAG release-2.8.1
	OPTIONS
		"SDL2IMAGE_SAMPLES OFF"
		"SDL2IMAGE_TESTS OFF"
		"SDL2IMAGE_SHARED ON"
		"SDL2IMAGE_STATIC OFF"
)

CPMAddPackage(
	NAME SDL2_mixer
	GITHUB_REPOSITORY libsdl-org/SDL_mixer
	GIT_TAG release-2.6.3
	OPTIONS
		"SDL2MIXER_SAMPLES OFF"
		"SDL2MIXER_MIDI OFF"
		"SDL2MIXER_FLAC OFF"
		"SDL2MIXER_MOD OFF"
		"SDL2MIXER_INSTALL OFF"
		"SDL2MIXER_SHARED ON"
		"SDL2MIXER_STATIC OFF"
)

CPMAddPackage(
	NAME SDL2_ttf
	GITHUB_REPOSITORY libsdl-org/SDL_ttf
	GIT_TAG release-2.20.2
	OPTIONS
		"SDL2TTF_SAMPLES OFF"
		"SDL2TTF_INSTALL OFF"
		"SDL2TTF_SHARED ON"
		"SDL2TTF_STATIC OFF"
)

set(IS_DEBUG $<BOOL:$<CONFIG:Debug>>)

if (NOT ${IS_DEBUG})
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

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE 
	${SDL2_INCLUDE_DIRS}
	${SDL2_IMAGE_INCLUDE_DIRS}
	${SDL2_TTF_INCLUDE_DIRS}
	${SDL2_MIXER_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
	SDL2::SDL2
	SDL2::SDL2main
	SDL2_image::SDL2_image
	SDL2_ttf::SDL2_ttf
	SDL2_mixer::SDL2_mixer
)

install(TARGETS ${PROJECT_NAME} 
	DESTINATION .
	RUNTIME_DEPENDENCIES DIRECTORIES ${SDL2_LIBRARY_DIRS}
)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets DESTINATION .)

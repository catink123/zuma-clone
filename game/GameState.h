#pragma once
#include "../engine/common.h"
#include "../engine/Audio.h"
#include "../engine/SoundManager.h"
#include <map>
#include <functional>

struct MouseState {
	vec2 previous_mouse_pos = vec2();
	vec2 mouse_pos = vec2();
	bool mouse_on_ui = false;
	bool is_lmb_down = false;
	bool is_rmb_down = false;

	bool lmb_just_pressed = false;
	bool lmb_just_unpressed = false;
	bool rmb_just_pressed = false;
	bool rmb_just_unpressed = false;

	void reset_frame_state() {
		lmb_just_pressed = false;
		lmb_just_unpressed = false;
		rmb_just_pressed = false;
		rmb_just_unpressed = false;
	}
};

struct KeyboardState {
	const Uint8* keys = nullptr;

	bool key_just_pressed = false;
	bool key_just_unpressed = false;

	void reset_frame_state() {
		key_just_pressed = false;
		key_just_unpressed = false;
	}
};

struct RendererState {
	vec2 window_size = vec2(1280, 720);
	float scaling = 1;
	bool is_fullscreen = false;
};

enum GameSection {
	None,
	InMenu,
	InLevel,
	DeathScreen,
	WinScreen,
	LevelSelection,
	InSettings
};

class GameState {
	GameSection section = None;
public:
	bool is_exiting = false;
	std::map<GameSection, Audio&> section_music;

	std::function<void(std::function<void(void)>, float)> fade_in;
	std::function<void(std::function<void(void)>, float)> fade_out;

	uint game_score = 0;
	MouseState mouse_state;
	KeyboardState keyboard_state;
	RendererState renderer_state;

	const GameSection& get_section() const { return section; }
	void set_section(const GameSection& new_section) {
		section = new_section;
		if (section_music.find(new_section) != section_music.end())
			SoundManager::set_music(section_music.at(section));
		else
			SoundManager::stop_music();
	}

	void exit() { is_exiting = true; }
	void save_settings() {
		//auto io = SDL_RWFromFile("settings.dat", "wb");
		//SDL_RWwrite(io, );
		// to be filled...
	}

	void load_settings() {
		// to be filled...
	}
};

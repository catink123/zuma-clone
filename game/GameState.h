#pragma once
#include "../engine/common.h"
#include "Level.h"

struct MouseState {
	vec2 previous_mouse_pos = vec2();
	vec2 mouse_pos = vec2();
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
	InLevel
};

class GameState {
public:
	bool is_exiting = false;

	Level* level = nullptr;
	GameSection section = None;
	MouseState mouse_state;
	KeyboardState keyboard_state;
	RendererState renderer_state;

	void exit() { is_exiting = true; }
};

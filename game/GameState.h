#pragma once
#include "../engine/common.h"
#include "Level.h"

struct MouseState {
	vec2 mouse_pos = vec2();
	bool is_lmb_pressed = false;
	bool is_rmb_pressed = false;
};

struct RendererState {
	vec2 window_size = vec2(1280, 720);
	float scaling = 1;
};

struct GameState {
	Level* level = nullptr;
	MouseState mouse_state;
	RendererState renderer_state;
};

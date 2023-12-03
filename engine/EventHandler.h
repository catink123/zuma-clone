#pragma once
#include <SDL.h>
#include "../game/GameState.h"

class EventHandler {
public:
	EventHandler() {}
	virtual void handle_events(SDL_Event* event, GameState& game_state) = 0;
};

class MouseHandler : public EventHandler {
public:
	void handle_events(SDL_Event* event, GameState& game_state) override {
		switch (event->type) {
			case SDL_MOUSEMOTION: {
				int mouse_x;
				int mouse_y;
				SDL_GetMouseState(&mouse_x, &mouse_y);
				game_state.mouse_state.previous_mouse_pos = game_state.mouse_state.mouse_pos;
				game_state.mouse_state.mouse_pos =
					vec2(
						static_cast<float>(mouse_x) / game_state.renderer_state.scaling,
						static_cast<float>(mouse_y) / game_state.renderer_state.scaling
					);
				break;
			}
			case SDL_MOUSEBUTTONDOWN: {
				uint bitmask = SDL_GetMouseState(nullptr, nullptr);
				if (bitmask & SDL_BUTTON(1)) {
					game_state.mouse_state.is_lmb_down = true;
					game_state.mouse_state.lmb_just_pressed = true;
				}
				if (bitmask & SDL_BUTTON(3)) {
					game_state.mouse_state.is_rmb_down = true;
					game_state.mouse_state.rmb_just_pressed = true;
				}
				break;
			}
			case SDL_MOUSEBUTTONUP: {
				uint bitmask = SDL_GetMouseState(nullptr, nullptr);
				if (!(bitmask & SDL_BUTTON(1))) {
					game_state.mouse_state.is_lmb_down = false;
					game_state.mouse_state.lmb_just_unpressed = true;
				}
				if (!(bitmask & SDL_BUTTON(3))) {
					game_state.mouse_state.is_rmb_down = false;
					game_state.mouse_state.rmb_just_unpressed = true;
				}
				break;
			}
		}
	}
};

class KeyboardHandler : public EventHandler {
public:
	void handle_events(SDL_Event* event, GameState& game_state) override {
		switch (event->type) {
			case SDL_KEYDOWN: {
				const Uint8* key_states = SDL_GetKeyboardState(nullptr);
				game_state.keyboard_state.keys = key_states;
				game_state.keyboard_state.key_just_unpressed = true;
				break;
			}
			case SDL_KEYUP: {
				const Uint8* key_states = SDL_GetKeyboardState(nullptr);
				game_state.keyboard_state.keys = key_states;
				game_state.keyboard_state.key_just_pressed = true;
				break;
			}
		}
	}
};

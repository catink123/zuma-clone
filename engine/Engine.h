#pragma once
#include <SDL.h>
#undef main
#include <iostream>
#include <unordered_map>
#include <string>
#include "AssetManager.h"
#include "EventHandler.h"
#include "basics.h"
#include "EntityManager.h"
#include "Sprite.h"
#include "../game/Player.h"
#include "../game/GameState.h"

using namespace std;

typedef unordered_map<string, shared_ptr<Drawable>> entity_map;

class EntityNonexistentException : public exception {};

class Engine {
	static const int WIDTH = 1280;
	static const int HEIGHT = 720;

	float max_frame_time = 0.0333333F;

	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	GameState game_state;
	float last_time = 0;

	Timer* keyboard_timer = nullptr;

	void load_media();
	void draw();
	void update();
	void poll_events(bool& quit);
	void change_window_size(int w, int h);

public:
	shared_ptr<AssetManager> asset_manager;
	shared_ptr<EntityManager> entity_manager;
	vector<EventHandler*> event_handlers;

	Engine();
	~Engine();

	void run_loop();
	void add_event_handler(EventHandler* event_handler);
	void set_scale(float scale);

	void set_fullscreen(bool state);
};
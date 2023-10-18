#pragma once
#include <SDL.h>
#undef main
#include <iostream>
#include <unordered_map>
#include <string>
#include "AssetManager.h"
#include "EventHandler.h"
#include "basics.h"
#include "../game/GameState.h"

using namespace std;

typedef unordered_map<string, shared_ptr<Drawable>> drawable_map;

class Engine {
	static const int WIDTH = 1280;
	static const int HEIGHT = 720;

	float max_frame_time = 0.0333333F;

	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	GameState game_state;
	float last_time = 0;

	void load_media();
	void draw();
	void update();
	void poll_events(bool& quit);
	void change_window_size(int w, int h);

public:
	shared_ptr<AssetManager> asset_manager;
	vector<EventHandler*> event_handlers;
	vector<shared_ptr<Drawable>> drawables;
	drawable_map drawable_map;

	Engine();
	~Engine();

	void run_loop();
	void add_drawable(string id, shared_ptr<Drawable> drawable);
	void remove_drawable(string id);
	void add_event_handler(EventHandler* event_handler);
	void add_drawable_raw(shared_ptr<Drawable> drawable);
	void set_scale(float scale);

	template <typename T>
	vector<shared_ptr<T>> get_drawables_by_type();
};
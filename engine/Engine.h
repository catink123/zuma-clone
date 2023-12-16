#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
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
#include "UI.h"
#include "UIElements/Button.h"
#include "UIElements/FlexContainer.h"
#include "../game/Level.h"

using namespace std;

typedef unordered_map<string, shared_ptr<Drawable>> entity_map_t;

void create_level_ui(shared_ptr<EntityManager> entity_manager, shared_ptr<AssetManager> asset_manager, SDL_Renderer* renderer);

class Engine {
	float max_frame_time = 0.0333333F;

	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	GameState game_state;
	float last_time = 0;

	Timer* keyboard_timer = nullptr;

	void prepare();
	void draw();
	void update();
	void poll_events();
	void change_window_size(int w, int h);

	void prepare_death_ui();
	void prepare_win_ui();
	void prepare_menu_ui();
	void prepare_level_select_ui();
	void prepare_settings_ui();

public:
	static const int WIDTH = 1280;
	static const int HEIGHT = 720;

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
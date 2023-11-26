#include "Engine.h"

Engine::Engine() {
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL couldn't initialize! Error: %s\n", SDL_GetError());
		return;
	}
	
	set_scale(1.25);

	window = SDL_CreateWindow(
		"Zuma", 
		SDL_WINDOWPOS_CENTERED, 
		SDL_WINDOWPOS_CENTERED, 
		static_cast<int>(Engine::WIDTH * this->game_state.renderer_state.scaling), 
		static_cast<int>(Engine::HEIGHT * this->game_state.renderer_state.scaling), 
		SDL_WINDOW_SHOWN
	);

	if (window == nullptr) {
		printf("Couldn't create the window! Error: %s\n", SDL_GetError());
		return;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	if (renderer == nullptr) {
		printf("Couldn't create the renderer! Error: %s\n", SDL_GetError());
		return;
	}

	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

	// Initialize SDL_Image
	int img_flags = IMG_INIT_PNG;
	if (!(IMG_Init(img_flags) & img_flags)) {
		printf("Couldn't initialize SDL_image! Error: %s\n", IMG_GetError());
		return;
	}

	SDL_DisplayMode current_display_mode;
	SDL_GetCurrentDisplayMode(0, &current_display_mode);
	max_frame_time = 1 / static_cast<float>(current_display_mode.refresh_rate);

	asset_manager = make_shared<AssetManager>();
	entity_manager = make_shared<EntityManager>();
	load_media();

	run_loop();
}

Engine::~Engine() {
	asset_manager.reset();
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();
}

void Engine::run_loop() {
	bool quit = false;
	while (quit == false) {
		poll_events(quit);

		SDL_RenderClear(renderer);
		update();
		draw();
		SDL_RenderPresent(renderer);
	}
}

void Engine::poll_events(bool& quit) {
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		if (e.type == SDL_QUIT)
			quit = true;
		for (EventHandler* handler : event_handlers) {
			handler->handle_events(&e, game_state);
		}
	}
}

void Engine::add_event_handler(EventHandler* event_handler) {
	event_handlers.push_back(event_handler);
}

void Engine::draw() {
	for (shared_ptr<Drawable> dr : entity_manager->entities) {
		dr->draw(renderer, game_state.renderer_state);
	}
}

void Engine::update() {
	float current_time = static_cast<float>(SDL_GetTicks()) / 1000.0F;
	float delta = current_time - last_time;
	last_time = current_time;

	if (delta < max_frame_time) {
		SDL_Delay(static_cast<uint>((max_frame_time - delta) * 1000));
	}

	vector<shared_ptr<Updatable>> updatables = entity_manager->get_entities_by_type<Updatable>();
	for (auto updatable : updatables) {
		updatable->update(delta, game_state);
	}
}

void Engine::load_media() {
	asset_manager->load_texture("player_normal", "assets/player_normal.png", renderer);
	asset_manager->load_texture("player_action", "assets/player_action.png", renderer);
	asset_manager->load_texture("ball_red", "assets/ball_red.png", renderer);
	asset_manager->load_texture("ball_blue", "assets/ball_blue.png", renderer);
	asset_manager->load_texture("ball_green", "assets/ball_green.png", renderer);
	asset_manager->load_texture("ball_purple", "assets/ball_purple.png", renderer);
	asset_manager->load_texture("ball_yellow", "assets/ball_yellow.png", renderer);
	asset_manager->load_texture("ball_gray", "assets/ball_gray.png", renderer);
	asset_manager->load_texture("ball_sheen", "assets/ball_sheen.png", renderer);

	vector<vec2> track_points = {
		vec2(0, 0),
		vec2(320, 360),
		vec2(700, 620),
		vec2(880, 620),
		vec2(960, 360),
		vec2(1280, 0)
	};

	auto ball_track = dynamic_pointer_cast<BallTrack>(
		entity_manager->add_entity(
			"ball_track",
			make_shared<BallTrack>(
				track_points,
				asset_manager
			)
		)
	);

	entity_manager->add_entity(
		"player", 
		make_shared<Player>(
			asset_manager->get_texture("player_normal"), 
			asset_manager->get_texture("player_action"),
			asset_manager,
			entity_manager,
			ball_track
		)
	);

	shared_ptr<Player> player = entity_manager->get_entity_by_name<Player>("player");
	if (player != nullptr) {
		player->local_transform.scale = 0.75;
		player->global_transform.position = vec2(WIDTH / 2, HEIGHT / 2);
	}

	add_event_handler(new MouseHandler());
}

void Engine::change_window_size(int w, int h) {
	SDL_SetWindowSize(window, w, h);
}

void Engine::set_scale(float scale) {
	game_state.renderer_state.scaling = scale;
	int w = static_cast<int>(WIDTH * scale);
	int h = static_cast<int>(HEIGHT * scale);
	change_window_size(w, h);
}

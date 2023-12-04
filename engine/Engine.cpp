#include "Engine.h"

Engine::Engine() {
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL couldn't initialize! Error: %s\n", SDL_GetError());
		return;
	}
	
	set_scale(1);

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

	if (TTF_Init() == -1) {
		printf("Couldn't initialize SDL_ttf! Error: %s\n", TTF_GetError());
	}

	SDL_DisplayMode current_display_mode;
	SDL_GetCurrentDisplayMode(0, &current_display_mode);
	max_frame_time = 1 / static_cast<float>(current_display_mode.refresh_rate);

	asset_manager = make_shared<AssetManager>();
	entity_manager = make_shared<EntityManager>();
	load_media();

	//set_fullscreen(true);
}

Engine::~Engine() {
	for (auto handler : event_handlers)
		delete handler;

	SDL_DestroyWindow(window);
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

void Engine::run_loop() {
	while (!game_state.is_exiting) {
		entity_manager->delete_scheduled();
		poll_events();

		SDL_RenderClear(renderer);
		update();
		draw();
		SDL_RenderPresent(renderer);
	}
}

void Engine::poll_events() {
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		if (e.type == SDL_QUIT)
			game_state.is_exiting = true;
		for (EventHandler* handler : event_handlers) {
			handler->handle_events(&e, game_state);
		}
	}
}

void Engine::add_event_handler(EventHandler* event_handler) {
	event_handlers.push_back(event_handler);
}

void Engine::draw() {
	for (shared_ptr<Drawable> dr : entity_manager->get_entities_by_section(game_state.section)) {
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

	vector<shared_ptr<Updatable>> updatables = entity_manager->get_entities_by_section_and_type<Updatable>(game_state.section);
	for (auto updatable : updatables) {
		updatable->update(delta, game_state);
	}

	// process keyboard
	if (game_state.keyboard_state.keys && game_state.keyboard_state.keys[SDL_SCANCODE_F]) {
		if (keyboard_timer == nullptr) {
			keyboard_timer = new Timer(1);
			keyboard_timer->set_progress(1);
		}

		if (keyboard_timer->is_done()) {
			set_fullscreen(!game_state.renderer_state.is_fullscreen);
			keyboard_timer->reset(true);
		}
	}
	else {
		if (keyboard_timer)
			delete keyboard_timer;

		keyboard_timer = nullptr;
	}

	if (keyboard_timer)
		keyboard_timer->update(delta, game_state);

	if (game_state.keyboard_state.keys && game_state.keyboard_state.keys[SDL_SCANCODE_ESCAPE]) {
		game_state.section = InMenu;
	}

	game_state.mouse_state.reset_frame_state();
	game_state.keyboard_state.reset_frame_state();
}

void Engine::load_media() {
	asset_manager->load_texture("player_normal", "assets/player_normal.catex", renderer);
	asset_manager->load_texture("player_action", "assets/player_action.catex", renderer);
	asset_manager->load_texture("ball_red", "assets/ball_red.catex", renderer);
	asset_manager->load_texture("ball_blue", "assets/ball_blue.catex", renderer);
	asset_manager->load_texture("ball_green", "assets/ball_green.catex", renderer);
	asset_manager->load_texture("ball_purple", "assets/ball_purple.catex", renderer);
	asset_manager->load_texture("ball_yellow", "assets/ball_yellow.catex", renderer);
	asset_manager->load_texture("ball_gray", "assets/ball_gray.catex", renderer);
	asset_manager->load_texture("ball_sheen", "assets/ball_sheen.catex", renderer);
	asset_manager->load_ui_texture("button", "assets/button.cauit", renderer);
	asset_manager->load_ui_texture("button_transparent", "assets/button_transparent.cauit", renderer);
	asset_manager->load_font("test24", "assets/Josefin.ttf");
	asset_manager->load_font("test14", "assets/Josefin.ttf", 14);

	asset_manager->load_ui_texture("medieval_button", "assets/medieval_button.cauit", renderer);
	asset_manager->load_font("medieval_button_font", "assets/BerkshireSwash-Regular.ttf", 24);

	vector<vec2> track_points = {
		vec2(0, 0),
		vec2(320, 360),
		vec2(700, 620),
		vec2(880, 620),
		vec2(960, 360),
		vec2(1280, 0)
	};

	auto ball_track =
		entity_manager->add_entity(
			"ball_track",
			make_shared<BallTrack>(
				track_points,
				asset_manager
			),
			InLevel
		);

	entity_manager->add_entity(
		"player", 
		make_shared<Player>(
			asset_manager->get_texture("player_normal"), 
			asset_manager->get_texture("player_action"),
			asset_manager,
			entity_manager,
			ball_track
		),
		InLevel
	);

	shared_ptr<Player> player = entity_manager->get_entity_by_name<Player>("player");
	if (player != nullptr) {
		player->local_transform.scale = 0.75;
		player->global_transform.position = vec2(WIDTH / 2, HEIGHT / 2);
	}

	add_event_handler(new MouseHandler());
	add_event_handler(new KeyboardHandler());
	
	auto ui = entity_manager->add_entity(
		"menu_ui",
		make_shared<UI>(renderer),
		InMenu
	);

	auto play_button =
		make_shared<Button>(
			"play_button",
			ui,
			&asset_manager->get_ui_texture("medieval_button"),
			"Play",
			&asset_manager->get_font("medieval_button_font"),
			SDL_Color({ 65, 45, 10 }),
			BoundingBox(25, 15),
			vec2(10, 10)
		);

	play_button->fit_content = true;

	play_button->add_event_listener(
		LMBUp, "change_to_inlevel", 
		[](GameState& game_state, UIElement* el) {
			game_state.section = InLevel;
		}
	);

	auto settings_button =
		make_shared<Button>(
			"settings_button",
			ui,
			&asset_manager->get_ui_texture("medieval_button"),
			"Settings",
			&asset_manager->get_font("medieval_button_font"),
			SDL_Color({ 65, 45, 10 }),
			BoundingBox(25, 15),
			vec2(10, 10)
		);
	settings_button->fit_content = true;

	auto exit_button =
		make_shared<Button>(
			"exit_button",
			ui,
			&asset_manager->get_ui_texture("medieval_button"),
			"Exit",
			&asset_manager->get_font("medieval_button_font"),
			SDL_Color({ 65, 45, 10 }),
			BoundingBox(25, 15),
			vec2(10, 10)
		);
	exit_button->fit_content = true;

	exit_button->add_event_listener(
		LMBUp,
		"exit_click",
		[](GameState& game_state, UIElement*) {
			game_state.exit();
		}
	);

	auto flex = 
		make_shared<FlexContainer>(
			"menu_flex",
			ui,
			BoundingBox(10),
			10,
			Y,
			vec2(),
			vec2(500, 300)
		);

	flex->fit_content = true;
	flex->add_children({ play_button, settings_button, exit_button });
	flex->alignment = FlexContainer::Alignment::Start;

	ui->root_element = flex;

	auto game_ui =
		entity_manager->add_entity(
			"game_ui",
			make_shared<UI>(renderer),
			InLevel
		);

	auto game_flex =
		make_shared<FlexContainer>(
			"game_strip",
			game_ui, BoundingBox(10),
			10, X,
			vec2(), vec2(1280, 0)
		);

	auto score =
		make_shared<Text>("score", game_ui, "Score: 0", &asset_manager->get_font("medieval_button_font"));

	auto pause_button =
		make_shared<Button>(
			"pause_button", game_ui, 
			&asset_manager->get_ui_texture("medieval_button"), 
			"Pause", 
			&asset_manager->get_font("medieval_button_font"), 
			SDL_Color({ 65, 45, 10 }), BoundingBox(20, 10)
		);

	pause_button->add_event_listener(
		LMBUp, "pause_game",
		[](GameState& game_state, auto) {
			game_state.section = InMenu;
		}
	);

	game_flex->add_children({ pause_button, score });
	game_flex->alignment = FlexContainer::Alignment::Middle;
	game_ui->root_element = game_flex;

	game_state.section = InLevel;
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

void Engine::set_fullscreen(bool state) {
	game_state.renderer_state.is_fullscreen = state;
	if (state) {
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

		SDL_DisplayMode current_display_mode;
		SDL_GetCurrentDisplayMode(0, &current_display_mode);
		// calculate scaling factor and set it
		set_scale(static_cast<float>(current_display_mode.h) / static_cast<float>(Engine::HEIGHT));
	}
	else {
		SDL_SetWindowFullscreen(window, 0);

		set_scale(1);
	}
}

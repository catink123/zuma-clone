#include "Engine.h"
#include "Fade.h"

void create_level_ui(shared_ptr<EntityManager> entity_manager, shared_ptr<AssetManager> asset_manager, SDL_Renderer* renderer) {
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
			20, X,
			vec2(), vec2(1280, 0)
		);

	auto score =
		make_shared<Text>("score", game_ui, "Score: 0", &asset_manager->get_font("medieval_button_font"), SDL_Color({ 255, 255, 255 }));
	score->fit_content = true;

	auto pause_button =
		make_shared<Button>(
			"pause_button", game_ui, 
			&asset_manager->get_ui_texture("medieval_button"), 
			"Exit", 
			&asset_manager->get_font("medieval_button_font"), 
			SDL_Color({ 65, 45, 10 }), BoundingBox(20, 10)
		);

	pause_button->add_event_listener(
		LMBUp, "pause_game",
		[entity_manager](GameState& game_state, auto) {
			game_state.fade_in([entity_manager, &game_state]() {
				entity_manager->schedule_to_delete("level");
				game_state.set_section(InMenu);
				game_state.fade_out([](){}, Fade::DURATION);
			}, Fade::DURATION);
		}
	);

	game_flex->add_children({ pause_button, score });
	game_flex->alignment = FlexContainer::Alignment::Middle;
	game_ui->root_element = game_flex;
}

Engine::Engine() {
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		printf("SDL couldn't initialize! Error: %s\n", SDL_GetError());
		return;
	}
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL initialized.\n");

	game_state.load_settings();
	
	window = SDL_CreateWindow(
		"Catink Adventures", 
		SDL_WINDOWPOS_CENTERED, 
		SDL_WINDOWPOS_CENTERED, 
		static_cast<int>(Engine::WIDTH * this->game_state.renderer_state.scaling), 
		static_cast<int>(Engine::HEIGHT * this->game_state.renderer_state.scaling), 
		SDL_WINDOW_SHOWN
	);

	if (window == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create the window! Error: %s\n", SDL_GetError());
		return;
	}
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Window created with scale %f.\n", game_state.renderer_state.scaling);

	set_fullscreen(game_state.renderer_state.is_fullscreen);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	if (renderer == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create the renderer! Error: %s\n", SDL_GetError());
		return;
	}
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Renderer initialized.\n");

	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

	// Initialize SDL_Image
	int img_flags = IMG_INIT_PNG;
	if (!(IMG_Init(img_flags) & img_flags)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL_image! Error: %s\n", IMG_GetError());
		return;
	}
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL_image initialized.\n");

	if (TTF_Init() == -1) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL_ttf! Error: %s\n", TTF_GetError());
	}
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL_ttf initialized.\n");

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initializer SDL_mixer! Error: %s\n", Mix_GetError());
		return;
	}

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL_mixer initialized.\n");

	SDL_DisplayMode current_display_mode;
	SDL_GetCurrentDisplayMode(0, &current_display_mode);
	max_frame_time = 1 / static_cast<float>(current_display_mode.refresh_rate);
	SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "Detected monitor refresh rate: %d.\n", current_display_mode.refresh_rate);

	asset_manager = make_shared<AssetManager>();
	entity_manager = make_shared<EntityManager>();
	prepare();
}

Engine::~Engine() {
	for (auto handler : event_handlers)
		delete handler;

	SDL_DestroyWindow(window);
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Window destroyed.\n");
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
	for (shared_ptr<Drawable> dr : entity_manager->get_entities_by_section(game_state.get_section())) {
		dr->draw(renderer, game_state.renderer_state);
	}

	for (shared_ptr<Drawable> dr : entity_manager->get_entities_by_section(None)) {
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

	// reset mouse_on_ui state to prepare for the next UI update
	game_state.mouse_state.mouse_on_ui = false;

	vector<shared_ptr<Updatable>> updatables = entity_manager->get_entities_by_section_and_type<Updatable>(game_state.get_section());
	for (auto updatable : updatables) {
		updatable->update(delta, game_state);
	}

	vector<shared_ptr<Updatable>> none_updatables = entity_manager->get_entities_by_section_and_type<Updatable>(None);
	for (auto none_upd : none_updatables) {
		none_upd->update(delta, game_state);
	}

	// update death screen text
	if (game_state.get_section() == DeathScreen) {
		auto death_ui = entity_manager->get_entity_by_name<UI>("death_ui");
		auto text = dynamic_pointer_cast<Text>(death_ui->root_element->children[0]);
		text->set_content(string("You failed. Score: ") + to_string(game_state.game_score));
	}

	// update win screen text
	if (game_state.get_section() == WinScreen) {
		auto win_ui = entity_manager->get_entity_by_name<UI>("win_ui");
		auto text = dynamic_pointer_cast<Text>(win_ui->root_element->children[0]);
		text->set_content(string("You won! Score: ") + to_string(game_state.game_score));
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
		game_state.set_section(InMenu);
	}

	game_state.mouse_state.reset_frame_state();
	game_state.keyboard_state.reset_frame_state();
}

void Engine::prepare_menu_ui() {
	entity_manager->add_entity(
		"menu_bg",
		make_shared<Sprite>(&asset_manager->get_texture("menu_bg"), 0, 1, 0, nullopt, vec2(WINDOW_WIDTH, WINDOW_HEIGHT)),
		InMenu
	);

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

	play_button->add_event_listener(
		LMBUp, "change_to_inlevel", 
		[](GameState& game_state, UIElement* el) {
			game_state.fade_in([&]() {
				game_state.set_section(LevelSelection);
				game_state.fade_out([]() {}, 0.25);
			}, 0.25);
		}
	);
	play_button->fit_content = true;

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

	settings_button->add_event_listener(
		LMBUp, "change_to_inlevel", 
		[](GameState& game_state, UIElement* el) {
			game_state.fade_in([&]() {
				game_state.set_section(InSettings);
				game_state.fade_out([]() {}, 0.25);
			}, 0.25);
		}
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

	exit_button->add_event_listener(
		LMBUp,
		"exit_click",
		[](GameState& game_state, UIElement*) {
			game_state.fade_in([&]() {
				game_state.exit();
			}, 3.0);
		}
	);
	exit_button->fit_content = true;

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

	flex->add_children({ play_button, settings_button, exit_button });
	flex->fit_content = true;
	flex->alignment = FlexContainer::Alignment::Middle;
	flex->update_layout(true);
	const vec2& flex_dims = flex->get_dimensions();
	flex->set_position(vec2(WINDOW_WIDTH / 2 - flex_dims.x / 2, WINDOW_HEIGHT - 100 - flex_dims.y));

	ui->root_element = flex;
}

void Engine::prepare_death_ui() {
	entity_manager->add_entity(
		"death_bg",
		make_shared<Sprite>(&asset_manager->get_texture("death_screen_bg"), 0, 1, 0, nullopt, vec2(WINDOW_WIDTH, WINDOW_HEIGHT)),
		DeathScreen
	);

	auto death_ui = entity_manager->add_entity(
		"death_ui",
		make_shared<UI>(
			renderer
		),
		DeathScreen
	);

	auto death_flex = 
		make_shared<FlexContainer>(
			"death_flex",
			death_ui,
			BoundingBox(10),
			10,
			Y,
			vec2(),
			vec2(500, 0)
		);

	death_flex->fit_content = true;
	death_flex->alignment = FlexContainer::Alignment::Middle;

	death_ui->root_element = death_flex;
	
	auto death_score_text = 
		make_shared<Text>("death_score", death_ui, "You failed! Score: xxxx", &asset_manager->get_font("medieval_button_font_large"), SDL_Color({ 255, 255, 255 }));

	auto death_go_back =
		make_shared<Button>(
			"exit_button",
			death_ui,
			&asset_manager->get_ui_texture("medieval_button"),
			"Go to Menu",
			&asset_manager->get_font("medieval_button_font"),
			SDL_Color({ 65, 45, 10 }),
			BoundingBox(25, 15),
			vec2(10, 10)
		);
	death_go_back->fit_content = true;

	death_go_back->add_event_listener(LMBUp, "go_to_menu", [](GameState& gs, auto) {
		gs.fade_in([&]() {
			gs.set_section(InMenu);
			gs.fade_out([]() {}, 0.25F);
		}, 0.25F);
	});

	death_flex->add_children({ death_score_text, death_go_back });

	// center on screen

	death_flex->update_layout(true);
	const vec2& df_dims = death_flex->get_dimensions();

	death_flex->set_position(vec2(WINDOW_WIDTH / 2 - df_dims.x / 2, WINDOW_HEIGHT / 2 + df_dims.y / 2));
}

void Engine::prepare_win_ui() {
	auto win_ui = entity_manager->add_entity(
		"win_ui",
		make_shared<UI>(
			renderer
		),
		WinScreen
	);

	auto win_flex = 
		make_shared<FlexContainer>(
			"win_flex",
			win_ui,
			BoundingBox(10),
			10,
			Y,
			vec2(),
			vec2(500, 0)
		);

	win_flex->fit_content = true;
	win_flex->alignment = FlexContainer::Alignment::Middle;

	win_ui->root_element = win_flex;
	
	auto win_score_text = 
		make_shared<Text>("win_score", win_ui, "You won! Score: xxxx", &asset_manager->get_font("medieval_button_font_large"), SDL_Color({ 0, 0, 0 }));

	auto win_go_back =
		make_shared<Button>(
			"exit_button",
			win_ui,
			&asset_manager->get_ui_texture("medieval_button"),
			"Go to Menu",
			&asset_manager->get_font("medieval_button_font"),
			SDL_Color({ 65, 45, 10 }),
			BoundingBox(25, 15),
			vec2(10, 10)
		);
	win_go_back->fit_content = true;

	win_go_back->add_event_listener(LMBUp, "go_to_menu", [](GameState& gs, auto) {
		gs.fade_in([&]() {
			gs.set_section(InMenu);
			gs.fade_out([]() {}, 0.25F);
		}, 0.25F);
	});

	win_flex->add_children({ win_score_text, win_go_back });

	// center on screen

	win_flex->update_layout(true);
	const vec2& wf_dims = win_flex->get_dimensions();

	win_flex->set_position(vec2(WINDOW_WIDTH / 2 - wf_dims.x / 2, WINDOW_HEIGHT / 2 - wf_dims.y / 2));
}

void Engine::prepare_level_select_ui() {
	entity_manager->add_entity(
		"level_select_bg",
		make_shared<Sprite>(&asset_manager->get_texture("level_select_bg"), 0, 1, 0, nullopt, vec2(WINDOW_WIDTH, WINDOW_HEIGHT)),
		LevelSelection
	);

	auto level_select_ui = entity_manager->add_entity(
		"level_select_ui",
		make_shared<UI>(
			renderer
		),
		LevelSelection
	);

	auto ls_flex = 
		make_shared<FlexContainer>(
			"ls_flex",
			level_select_ui,
			BoundingBox(10),
			10,
			Y,
			vec2(),
			vec2(500, 200)
		);

	ls_flex->fit_content = true;
	ls_flex->alignment = FlexContainer::Alignment::Middle;

	level_select_ui->root_element = ls_flex;

	auto ls_back =
		make_shared<Button>(
			"ls_back",
			level_select_ui,
			&asset_manager->get_ui_texture("medieval_button"),
			"Go Back",
			&asset_manager->get_font("medieval_button_font"),
			SDL_Color({ 65, 45, 10 }),
			BoundingBox(25, 15),
			vec2(10, 10)
		);

	ls_back->add_event_listener(LMBUp, "go_to_menu", [](GameState& gs, auto) {
		gs.fade_in([&]() {
			gs.set_section(InMenu);
			gs.fade_out([](){}, 0.25F);
		}, 0.25F);
	});
	
	auto ls1 =
		make_shared<Button>(
			"ls1",
			level_select_ui,
			&asset_manager->get_ui_texture("medieval_button"),
			"Level 1",
			&asset_manager->get_font("medieval_button_font"),
			SDL_Color({ 65, 45, 10 }),
			BoundingBox(25, 15),
			vec2(10, 10)
		);

	ls1->add_event_listener(LMBUp, "select_level", [=, this](GameState& gs, auto) {
		gs.fade_in([=, &gs, this]() {
			entity_manager->remove_entity("level");
			entity_manager->add_entity(
				"level",
				make_shared<Level>(
					&asset_manager->get_level_data("level1"),
					asset_manager, entity_manager, renderer, create_level_ui
				),
				InLevel
			);

			gs.set_section(InLevel);
			gs.fade_out([](){}, 1.0F);
		}, 1.0F);
	});
	
	auto ls2 =
		make_shared<Button>(
			"ls2",
			level_select_ui,
			&asset_manager->get_ui_texture("medieval_button"),
			"Level 2",
			&asset_manager->get_font("medieval_button_font"),
			SDL_Color({ 65, 45, 10 }),
			BoundingBox(25, 15),
			vec2(10, 10)
		);

	ls2->add_event_listener(LMBUp, "select_level", [=, this](GameState& gs, auto) {
		gs.fade_in([=, &gs, this]() {
			entity_manager->remove_entity("level");
			entity_manager->add_entity(
				"level",
				make_shared<Level>(
					&asset_manager->get_level_data("level2"),
					asset_manager, entity_manager, renderer, create_level_ui
				),
				InLevel
			);

			gs.set_section(InLevel);
			gs.fade_out([](){}, 1.0F);
		}, 1.0F);
	});

	ls_flex->add_children({ ls_back, ls1, ls2 });

	// center on screen

	ls_flex->update_layout(true);
	const vec2& lf_dims = ls_flex->get_dimensions();

	ls_flex->set_position(vec2(WINDOW_WIDTH / 2 - lf_dims.x / 2, WINDOW_HEIGHT / 2 - lf_dims.y / 2));
}

void Engine::prepare_settings_ui() {
	auto settings_ui = entity_manager->add_entity(
		"settings_ui",
		make_shared<UI>(renderer),
		InSettings
	);

	auto flex =
		make_shared<FlexContainer>(
			"settings_flex",
			settings_ui,
			BoundingBox(10),
			10,
			Y,
			0,
			vec2(300, 250)
		);
	flex->alignment = FlexContainer::Alignment::Middle;

	settings_ui->root_element = flex;

	auto caption_text =
		make_shared<Text>(
			"caption",
			settings_ui,
			"Settings",
			&asset_manager->get_font("medieval_button_font_large")
		);
	caption_text->fit_content = true;

	auto back_button =
		make_shared<Button>(
			"back_button",
			settings_ui,
			&asset_manager->get_ui_texture("medieval_button"),
			"Go Back",
			&asset_manager->get_font("medieval_button_font"),
			SDL_Color({ 65, 45, 10 }),
			BoundingBox(25, 15),
			vec2(10, 10)
		);

	back_button->add_event_listener(LMBUp, "go_to_menu", [](GameState& gs, auto) {
		gs.save_settings();
		gs.fade_in([&]() {
			gs.set_section(InMenu);
			gs.fade_out([](){}, 0.25F);
		}, 0.25F);
	});

	auto fullscreen_button =
		make_shared<Button>(
			"fullscreen_switch",
			settings_ui,
			&asset_manager->get_ui_texture("medieval_button"),
			"Go Fullscreen",
			&asset_manager->get_font("medieval_button_font"),
			SDL_Color({ 65, 45, 10 }),
			BoundingBox(25, 15),
			vec2(10, 10)
		);

	if (game_state.renderer_state.is_fullscreen)
		fullscreen_button->set_text_content("Go Windowed");

	fullscreen_button->add_event_listener(LMBUp, "fullscreen", [&](GameState& game_state, UIElement* el) {
		auto button = dynamic_cast<Button*>(el);

		bool& fs_state = game_state.renderer_state.is_fullscreen;

		set_fullscreen(!fs_state);

		if (fs_state) {
			button->set_text_content("Go Windowed");
		}
		else {
			button->set_text_content("Go Fullscreen");
		}
	});

	auto volume_text =
		make_shared<Text>(
			"volume",
			settings_ui,
			"Volume: 0",
			&asset_manager->get_font("medieval_button_font")
		);

	volume_text->set_content("Volume: " + to_string(static_cast<int>(floorf(SoundManager::get_volume() * 100))) + "%");
	volume_text->fit_content = true;

	auto volume_buttons_flex =
		make_shared<FlexContainer>(
			"vb_flex",
			settings_ui,
			BoundingBox(0),
			10,
			X
		);
	volume_buttons_flex->fit_content = true;

	auto add_volume_button =
		make_shared<Button>(
			"add_volume",
			settings_ui,
			&asset_manager->get_ui_texture("medieval_button"),
			"+",
			&asset_manager->get_font("medieval_button_font"),
			SDL_Color({ 65, 45, 10 }),
			BoundingBox(25, 15),
			vec2(10, 10)
		);

	auto sub_volume_button =
		make_shared<Button>(
			"sub_volume",
			settings_ui,
			&asset_manager->get_ui_texture("medieval_button"),
			"-",
			&asset_manager->get_font("medieval_button_font"),
			SDL_Color({ 65, 45, 10 }),
			BoundingBox(25, 15),
			vec2(10, 10)
		);

	add_volume_button->add_event_listener(LMBUp, "volume_change", [=](GameState& game_state, auto) {
		float new_volume = SoundManager::get_volume() + 0.05F;
		if (new_volume > 1)
			new_volume = 1;
		if (new_volume < 0)
			new_volume = 0;
		SoundManager::set_volume(new_volume);
		
		volume_text->set_content("Volume: " + to_string(static_cast<int>(floorf(new_volume * 100))) + "%");
	});

	sub_volume_button->add_event_listener(LMBUp, "volume_change", [=](GameState& game_state, auto) {
		float new_volume = SoundManager::get_volume() - 0.05;
		if (new_volume > 1)
			new_volume = 1;
		if (new_volume < 0)
			new_volume = 0;
		SoundManager::set_volume(new_volume);
		
		volume_text->set_content("Volume: " + to_string(static_cast<int>(floorf(new_volume * 100))) + "%");
	});

	volume_buttons_flex->add_children({ add_volume_button, sub_volume_button });

	auto scaling_text =
		make_shared<Text>(
			"scaling",
			settings_ui,
			"Window Scaling: 0",
			&asset_manager->get_font("medieval_button_font")
		);

	scaling_text->set_content("Window Scaling: " + to_string(static_cast<int>(floorf(game_state.renderer_state.saved_scaling * 100))) + "%");
	scaling_text->fit_content = true;

	auto scaling_buttons_flex =
		make_shared<FlexContainer>(
			"vb_flex",
			settings_ui,
			BoundingBox(0),
			10,
			X
		);
	scaling_buttons_flex->fit_content = true;

	auto add_scaling_button =
		make_shared<Button>(
			"add_scaling",
			settings_ui,
			&asset_manager->get_ui_texture("medieval_button"),
			"+",
			&asset_manager->get_font("medieval_button_font"),
			SDL_Color({ 65, 45, 10 }),
			BoundingBox(25, 15),
			vec2(10, 10)
		);

	auto sub_scaling_button =
		make_shared<Button>(
			"sub_scaling",
			settings_ui,
			&asset_manager->get_ui_texture("medieval_button"),
			"-",
			&asset_manager->get_font("medieval_button_font"),
			SDL_Color({ 65, 45, 10 }),
			BoundingBox(25, 15),
			vec2(10, 10)
		);

	add_scaling_button->add_event_listener(LMBUp, "scaling_change", [=, this](GameState& game_state, auto) {
		float new_scaling = clamp(game_state.renderer_state.scaling + 0.05F, 0.125F, 10.0F);
		//if (new_scaling > 10)
		//	new_scaling = 10;
		//if (new_scaling < 0.125)
		//	new_scaling = 0.125;

		game_state.renderer_state.saved_scaling = new_scaling;
		set_scale(new_scaling);
		
		scaling_text->set_content("Window Scaling: " + to_string(static_cast<int>(floorf(new_scaling * 100))) + "%");
	});

	sub_scaling_button->add_event_listener(LMBUp, "scaling_change", [=, this](GameState& game_state, auto) {
		float new_scaling = clamp(game_state.renderer_state.scaling - 0.05F, 0.125F, 10.0F);
		//if (new_scaling > 10)
		//	new_scaling = 10;
		//if (new_scaling < 0.125)
		//	new_scaling = 0.125;

		game_state.renderer_state.saved_scaling = new_scaling;
		set_scale(new_scaling);
		
		scaling_text->set_content("Window Scaling: " + to_string(static_cast<int>(floorf(new_scaling * 100))) + "%");
	});

	scaling_buttons_flex->add_children({ add_scaling_button, sub_scaling_button });

	flex->add_children({ caption_text, back_button, fullscreen_button, volume_text, volume_buttons_flex, scaling_text, scaling_buttons_flex });

	flex->update_layout(true);

	auto flex_dims = flex->get_dimensions();

	flex->set_position(vec2(WINDOW_WIDTH / 2 - flex_dims.x / 2, WINDOW_HEIGHT / 2 - flex_dims.y / 2));
}

void Engine::prepare() {
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Loading game assets...\n");
	asset_manager->load_texture("player_normal", "assets/player_normal.catex", renderer);
	asset_manager->load_texture("player_action", "assets/player_action.catex", renderer);
	asset_manager->load_texture("ball_red", "assets/ball_red.catex", renderer);
	asset_manager->load_texture("ball_blue", "assets/ball_blue.catex", renderer);
	asset_manager->load_texture("ball_green", "assets/ball_green.catex", renderer);
	asset_manager->load_texture("ball_purple", "assets/ball_purple.catex", renderer);
	asset_manager->load_texture("ball_yellow", "assets/ball_yellow.catex", renderer);
	asset_manager->load_texture("ball_gray", "assets/ball_gray.catex", renderer);
	asset_manager->load_texture("ball_sheen", "assets/ball_sheen.catex", renderer);
	asset_manager->load_texture("ball_particle", "assets/ball_particle.catex", renderer);

	asset_manager->load_texture("black", "assets/black.catex", renderer);
	asset_manager->load_texture("death_window", "assets/death_window.catex", renderer);

	asset_manager->load_ui_texture("medieval_button", "assets/medieval_button.cauit", renderer);
	asset_manager->load_font("medieval_button_font", "assets/BerkshireSwash-Regular.ttf", 24);
	asset_manager->load_font("medieval_button_font_large", "assets/BerkshireSwash-Regular.ttf", 48);
	asset_manager->load_level_data("level1", "assets/level1.calev", renderer);
	asset_manager->load_level_data("level2", "assets/level2.calev", renderer);

	asset_manager->load_texture("death_screen_bg", "assets/death_screen_bg.catex", renderer);
	asset_manager->load_texture("level_select_bg", "assets/level_select_bg.catex", renderer);
	asset_manager->load_texture("menu_bg", "assets/menu_bg.catex", renderer);

	asset_manager->load_audio("ball_break", "assets/ball_break.wav", Sound);
	asset_manager->load_audio("ball_collision", "assets/ball_collision.wav", Sound);
	asset_manager->load_audio("ball_collision_pitched", "assets/ball_collision_pitched.wav", Sound);

	asset_manager->load_audio("main_menu", "assets/main_menu.mp3", Music);
	asset_manager->load_audio("death_song", "assets/death_song.mp3", Music);
	asset_manager->load_audio("level_song", "assets/level_song.mp3", Music);

	game_state.section_music = {
		{ InMenu, asset_manager->get_audio("main_menu") },
		{ DeathScreen, asset_manager->get_audio("death_song") },
		{ InLevel, asset_manager->get_audio("level_song") }
	};

	SoundManager::set_music_volume(SoundManager::MUSIC_VOLUME * SoundManager::get_volume());

	add_event_handler(new MouseHandler());
	add_event_handler(new KeyboardHandler());

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Creating entities.\n");

	// global fade
	
	auto fade = entity_manager->add_entity("fade",
		make_shared<Fade>(&asset_manager->get_texture("black")),
		None
	);

	prepare_menu_ui();
	prepare_death_ui();
	prepare_win_ui();
	prepare_level_select_ui();
	prepare_settings_ui();

	// set the default section to be InMenu
	game_state.set_section(InMenu);

	// forward fade methods to the global game state
	game_state.fade_out = [=](function<void(void)> callback, float duration = Fade::DURATION) { fade->fade_out(callback, duration); };
	game_state.fade_in = [=](function<void(void)> callback, float duration = Fade::DURATION) { fade->fade_in(callback, duration); };

	// startup fading
	fade->fade_out([](){});
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

		set_scale(game_state.renderer_state.saved_scaling);
	}
}

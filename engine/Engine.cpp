#include "../engine/Engine.h"
#include "../engine/basics.h"
#include "../engine/Sprite.h"
#include "../game/Player.h"

Engine::Engine() {
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL couldn't initialize! Error: %s\n", SDL_GetError());
		return;
	}
	
	window = SDL_CreateWindow("Zuma", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Engine::WIDTH, Engine::HEIGHT, SDL_WINDOW_SHOWN);

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
		draw();
		update();
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
	for (shared_ptr<Drawable> dr : drawables) {
		dr->draw(renderer, game_state.renderer_state);
	}
}

void Engine::update() {
	float current_time = static_cast<float>(SDL_GetTicks()) / 1000.0F;
	float delta = current_time - last_time;
	last_time = current_time;

	if (delta < max_frame_time) {
		delta = max_frame_time;
		SDL_Delay(static_cast<uint>((max_frame_time - delta) * 1000));
	}

	vector<shared_ptr<Animatable>> animatables = get_drawables_by_type<Animatable>();
	for (auto animatable : animatables) {
		animatable->update(delta);
	}

	vector<shared_ptr<Ball>> balls = get_drawables_by_type<Ball>();

	for (auto ball : balls) {
		ball->set_ball_angle(last_time * 100);
		ball->rotation = last_time * 10;
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

	set_scale(1.25);

	add_drawable("ball1", make_shared<Ball>(asset_manager, BallColor::Red, vec2(10, 10)));
	add_drawable("ball2", make_shared<Ball>(asset_manager, BallColor::Green, vec2(10 + 40, 10)));
	add_drawable("ball3", make_shared<Ball>(asset_manager, BallColor::Blue, vec2(10 + 80, 10)));
	add_drawable("ball4", make_shared<Ball>(asset_manager, BallColor::Yellow, vec2(10 + 120, 10)));
	add_drawable("ball5", make_shared<Ball>(asset_manager, BallColor::Purple, vec2(10 + 160, 10)));
	add_drawable("ball6", make_shared<Ball>(asset_manager, BallColor::Gray, vec2(10 + 200, 10)));

	auto balls = get_drawables_by_type<Ball>();
	for (auto ball : balls) {
		ball->set_ball_angle(90);
	}

	add_drawable(
		"player", 
		make_shared<Player>(
			make_pair<Texture*, Texture*>(
				asset_manager->get_texture("player_normal"), 
				asset_manager->get_texture("player_action")
			),
			&game_state
		)
	);

	shared_ptr<Player> player = dynamic_pointer_cast<Player>(drawable_map.at("player"));
	if (player != nullptr)
		player->position = vec2(WIDTH / 2, HEIGHT / 2);

	add_event_handler(new MouseHandler());
}

void Engine::add_drawable(string id, shared_ptr<Drawable> drawable) {
	// if the drawable with given id already exists, don't add anything
	if (drawable_map.find(id) != drawable_map.end()) return;
	drawable_map.insert({ id, drawable });
	add_drawable_raw(drawable);
}

void Engine::add_drawable_raw(shared_ptr<Drawable> drawable) {
	drawables.push_back(drawable);
}

void Engine::remove_drawable(string id) {
	if (drawable_map.find(id) == drawable_map.end()) return;
	drawable_map.erase(id);
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

template <typename T>
vector<shared_ptr<T>> Engine::get_drawables_by_type() {
	vector<shared_ptr<T>> result;

	for (shared_ptr<Drawable> dr : drawables) {
		shared_ptr<T> cast = dynamic_pointer_cast<T>(dr);
		if (cast != nullptr)
			result.push_back(cast);
	}

	return result;
}

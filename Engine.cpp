#include "Engine.h"
#include "basics.h"
#include "Sprite.h"
#include "Player.h"

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

	asset_manager = make_unique<AssetManager>();
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
	//auto t_data = asset_manager->get_texture("player_normal");
	//auto texture = t_data->get_texture();
	//SDL_Rect out_rect = t_data->get_rect(0, 0, 0.5);
	//SDL_RenderCopy(renderer, texture, NULL, &out_rect);
	for (shared_ptr<Drawable> dr : drawables) {
		dr->draw(renderer, game_state.renderer_state);
	}
}

void Engine::load_media() {
	asset_manager->load_texture("player_normal", "assets/player_normal.png", renderer);
	asset_manager->load_texture("player_action", "assets/player_action.png", renderer);

	set_scale(0.75);

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

#include "../engine/AssetManager.h"

AMAssetLoadException::AMAssetLoadException(const char* msg) : msg(msg) {}
const char* AMAssetLoadException::what() { return msg; }

AssetManager::AssetManager() {}

AssetManager::~AssetManager() {
	for (auto pair : textures) {
		SDL_Texture* asset = pair.second.get_raw();
		SDL_DestroyTexture(asset);
	}
}

Texture* AssetManager::get_texture(string id) {
	if (textures.find(id) == textures.end()) {
		throw AMAssetNotRegisteredException();
	}
	return &textures[id];
}

void AssetManager::load_texture(string id, string path, SDL_Renderer* renderer) {
	// Asset already loaded, no need to load it again
	if (textures.find(id) != textures.end()) return;

	SDL_Surface* surface = IMG_Load(path.c_str());
	if (surface == nullptr) {
		auto sdl_error = IMG_GetError();
		printf("Couldn't load image %s! Error: %s\n", path.c_str(), sdl_error);
		throw AMAssetLoadException(sdl_error);
	}

	SDL_Texture* texture = IMG_LoadTexture(renderer, path.c_str());
	if (texture == nullptr) {
		auto sdl_error = SDL_GetError();
		printf("Couldn't load image %s! Error: %s\n", path.c_str(), sdl_error);
		throw AMAssetLoadException(sdl_error);
	}

	Texture t_data(surface->w, surface->h, texture);

	SDL_FreeSurface(surface);

	textures.insert({ id, t_data });
}
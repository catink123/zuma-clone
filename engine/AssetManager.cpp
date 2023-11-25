#include "../engine/AssetManager.h"

AMAssetLoadException::AMAssetLoadException(const char* msg) : msg(msg) {}
const char* AMAssetLoadException::what() { return msg; }

AssetManager::AssetManager() {}

AssetManager::~AssetManager() {
	// get all raw texture pointer and destroy the textures
	for (auto pair : textures) {
		SDL_Texture* asset = pair.second.get_raw();
		SDL_DestroyTexture(asset);
	}
}

const Texture& AssetManager::get_texture(const string& id) const {
	// if the texture by the given ID isn't found, it's not registered, throw an exception
	if (textures.find(id) == textures.end()) {
		throw AMAssetNotRegisteredException();
	}
	return textures.at(id);
}

void AssetManager::load_texture(const string& id, const string& path, SDL_Renderer* renderer) {
	// the asset is already loaded, there's no need to load it again
	if (textures.find(id) != textures.end()) return;

	// construct the path string
	auto constructed_path = string(prefix) + path;
	auto c_path_str = constructed_path.c_str();

	SDL_Surface* surface = IMG_Load(c_path_str);
	// in case of an error (surface = nullptr) throw the error as an exception
	if (surface == nullptr) {
		auto sdl_error = IMG_GetError();
		printf("Couldn't load image %s! Error: %s\n", c_path_str, sdl_error);
		throw AMAssetLoadException(sdl_error);
	}

	SDL_Texture* texture = IMG_LoadTexture(renderer, c_path_str);
	// same as above error handling
	if (texture == nullptr) {
		auto sdl_error = SDL_GetError();
		printf("Couldn't load image %s! Error: %s\n", c_path_str, sdl_error);
		throw AMAssetLoadException(sdl_error);
	}

	// encapsulate raw texture pointer, it's width and height in a Texture object
	Texture t_data(surface->w, surface->h, texture);

	// after the texture is loaded, the surface is no longer needed, so it is freed
	SDL_FreeSurface(surface);

	textures.insert({ id, t_data });
}

void AssetManager::unload_texture(const string& id) {
	// if the texture wasn't found
	if (textures.find(id) == textures.end()) return;

	// we don't need the texture anymore, destroy it
	SDL_DestroyTexture(textures.at(id).get_raw());

	// erase the record of said texture
	textures.erase(id);
}
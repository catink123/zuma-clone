#pragma once
#include <SDL.h>
#undef main
#include <SDL_image.h>
#include <unordered_map>
#include <string>
#include "Texture.h"

using namespace std;

class AMAssetLoadException : public exception {
	const char* msg;
public:
	AMAssetLoadException(const char* msg);
	const char* what();
};

class AMAssetNotRegisteredException : public exception {};

// AssetManager is the central place for loading and retrieving textures and other assets
class AssetManager {
	unordered_map<string, Texture> textures;
public:
	AssetManager();
	~AssetManager();

	// gets the texture handle for the given texture
	const Texture& get_texture(const string& id) const;

	// loads an image from path or gets it from cache if it is already loaded
	void load_texture(const string& id, const string& path, SDL_Renderer* renderer);

	// unloads an image by it's ID
	void unload_texture(const string& id);
};
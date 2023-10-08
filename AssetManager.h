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

class AssetManager {
	unordered_map<string, Texture> textures;
public:
	AssetManager();
	~AssetManager();

	Texture* get_texture(string id);

	// Loads an image from path or gets it from cache if it is already loaded
	void load_texture(string id, string path, SDL_Renderer* renderer);
};
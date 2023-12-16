#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <bitset>
#include <deque>
#include <variant>
#include "Texture.h"
#include "UI.h"
#include "../game/LevelData.h"
#include "Audio.h"

#ifdef NDEBUG
#define prefix "./"
#else
#define prefix "../../../"
#endif

using namespace std;

class AMAssetLoadException : public exception {
	const char* msg;
public:
	AMAssetLoadException(const char* msg);
	const char* what();
};

class AMAssetNotRegisteredException : public exception {};

enum AssetType {
	ATTexture,
	ATUITexture,
	ATLevel
};

struct FontCreationException : public runtime_error {
	FontCreationException(const char* msg) : runtime_error(msg) {}
};

class Font {
	static const uint BASE_DPI = 96;
	string path;
	TTF_Font* font = nullptr;
	uint pt_size;
	float scaling = 1;

	void open_font(const float& font_scaling = 1) {
		if (font && scaling == font_scaling)
			return;

		scaling = font_scaling;
		destroy();

		uint resulting_dpi =
			static_cast<uint>(
				static_cast<float>(BASE_DPI) * scaling
			);

		font = TTF_OpenFontDPI(path.c_str(), pt_size, resulting_dpi, resulting_dpi);
		if (font == nullptr)
			throw FontCreationException(TTF_GetError());
	}

public:
	Font(const string& path, const uint& pt_size) : path(path), pt_size(pt_size) {
		open_font();
	}

	Texture* render(SDL_Renderer* renderer, const string& text, SDL_Color color, const float& font_scaling = 1) {
		open_font(font_scaling);
		SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
		return new Texture(surface->w, surface->h, texture);
	}

	void destroy() {
		if (font)
			TTF_CloseFont(font);
	}
};

// AssetManager is the central place for loading and retrieving textures and other assets
class AssetManager {
	unordered_map<string, Texture> textures;
	unordered_map<string, UITexture> ui_textures;
	unordered_map<string, LevelData> levels;
	unordered_map<string, Font> fonts;
	unordered_map<string, Audio> audio;

	static bool is_signature_valid(const unsigned char* signature_data);
	float convert_float_type(unsigned char* data);
	uint convert_uint_type(unsigned char* data);

public:
	AssetManager();
	~AssetManager();

	// gets the texture handle for the given texture id
	Texture& get_texture(const string& id);

	UITexture& get_ui_texture(const string& id);
	LevelData& get_level_data(const string& id);
	Font& get_font(const string& id);
	Audio& get_audio(const string& id);

	// loads an image from path or gets it from cache if it is already loaded
	void load_texture(const string& id, const string& path, SDL_Renderer* renderer);
	// unloads an image by it's ID
	void unload_texture(const string& id);

	void load_ui_texture(const string& id, const string& path, SDL_Renderer* renderer);
	void unload_ui_texture(const string& id);

	void load_level_data(const string& id, const string& path, SDL_Renderer* renderer);
	void unload_level_data(const string& id);

	void load_font(const string& id, const string& path, int font_size = 24);
	void unload_font(const string& id);

	void load_audio(const string& id, const string& path, AudioType audio_type = Sound);
	void unload_audio(const string& id);
};
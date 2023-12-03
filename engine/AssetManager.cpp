#include "../engine/AssetManager.h"

AMAssetLoadException::AMAssetLoadException(const char* msg) : msg(msg) {}
const char* AMAssetLoadException::what() { return msg; }

AssetManager::AssetManager() {}

AssetManager::~AssetManager() {
	// get all raw texture pointer and destroy the textures
	for (auto pair : textures)
		pair.second.destroy();
		//SDL_Texture* asset = pair.second.get_raw();
		//SDL_DestroyTexture(asset);

	for (auto pair : ui_textures)
		pair.second.destroy();

	for (auto pair : fonts)
		pair.second.destroy();
}

Texture& AssetManager::get_texture(const string& id) {
	// if the texture by the given ID isn't found, it's not registered, throw an exception
	if (textures.find(id) == textures.end()) {
		throw AMAssetNotRegisteredException();
	}
	return textures.at(id);
}

UITexture& AssetManager::get_ui_texture(const string& id) {
	if (ui_textures.find(id) == ui_textures.end())
		throw AMAssetNotRegisteredException();

	return ui_textures.at(id);
}

Level& AssetManager::get_level(const string& id) {
	if (levels.find(id) == levels.end())
		throw AMAssetNotRegisteredException();

	return levels.at(id);
}

Font& AssetManager::get_font(const string& id) {
	if (fonts.find(id) == fonts.end())
		throw AMAssetNotRegisteredException();

	return fonts.at(id);
}

bool AssetManager::is_signature_valid(const char* signature_data) {
	return string(signature_data).substr(0, 5) == "CAASS";
}

void AssetManager::load_texture(const string& id, const string& path, SDL_Renderer* renderer) {
	// the asset is already loaded, there's no need to load it again
	if (textures.find(id) != textures.end()) return;

	// construct the path string
	auto constructed_path = string(prefix) + path;
	auto c_path_str = constructed_path.c_str();

	// open file for reading in binary form
	SDL_RWops* io = SDL_RWFromFile(c_path_str, "rb");
	// allocate a C string for signature data
	char* signature_data = (char*)malloc(6 * sizeof(char));

	// if couldn't allocate, throw an error
	if (!signature_data) throw runtime_error("couldn't create allocate signature_data!");

	// if couldn't read first 6 bytes, log and throw an error
	if (SDL_RWread(io, signature_data, 6 * sizeof(char), 1) <= 0) {
		auto sdl_error = SDL_GetError();
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load texture %s! Error: %s\n", c_path_str, sdl_error);
		throw AMAssetLoadException(sdl_error);
	}

	// if the signature read isn't valid (first 5 bytes don't match the asset signature),
	// log and throw an error
	if (!is_signature_valid(signature_data)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Signature of loaded file is invalid!");
		throw AMAssetLoadException("invalid signature");
	}

	// get the asset type from the 6th byte (after the asset signature)
	auto asset_type = (AssetType)signature_data[5];

	// we don't need signature_data, free it
	free(signature_data);

	// if the asset type isn't a Texture, log and throw an error
	if (asset_type != AssetType::ATTexture) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Loading non-texture as texture!");
		throw AMAssetLoadException("invalid asset type");
	}

	// load the PNG data from the file's RWops
	SDL_Surface* surface = IMG_LoadTyped_RW(io, 1, "PNG");
	// in case of an error (surface = nullptr) throw the error as an exception
	if (surface == nullptr) {
		auto sdl_error = IMG_GetError();
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load image %s! Error: %s\n", c_path_str, sdl_error);
		throw AMAssetLoadException(sdl_error);
	}

	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	// same as above error handling
	if (texture == nullptr) {
		auto sdl_error = SDL_GetError();
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load image %s! Error: %s\n", c_path_str, sdl_error);
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

void AssetManager::load_ui_texture(const string& id, const string& path, SDL_Renderer* renderer) {
	// the asset is already loaded, there's no need to load it again
	if (ui_textures.find(id) != ui_textures.end()) return;

	// construct the path string
	auto constructed_path = string(prefix) + path;
	auto c_path_str = constructed_path.c_str();

	// open file for reading in binary form
	SDL_RWops* io = SDL_RWFromFile(c_path_str, "rb");
	// allocate a C string for signature data
	char* signature_data = (char*)malloc(17 * sizeof(char));

	// if couldn't allocate, throw an error
	if (!signature_data) throw runtime_error("couldn't create allocate signature_data!");

	// if couldn't read first 17 bytes, log and throw an error
	if (SDL_RWread(io, signature_data, 17 * sizeof(char), 1) <= 0) {
		auto sdl_error = SDL_GetError();
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load texture %s! Error: %s\n", c_path_str, sdl_error);
		throw AMAssetLoadException(sdl_error);
	}

	// if the signature read isn't valid (first 5 bytes don't match the asset signature),
	// log and throw an error
	if (!is_signature_valid(signature_data)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Signature of loaded file is invalid!");
		throw AMAssetLoadException("invalid signature");
	}

	// get the asset type from the 6th byte (after the asset signature)
	auto asset_type = (AssetType)signature_data[5];

	// if the asset type isn't a Texture, log and throw an error
	if (asset_type != AssetType::ATUITexture) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Loading non-texture as texture!");
		throw AMAssetLoadException("invalid asset type");
	}

	// load the PNG data from the file's RWops
	SDL_Surface* surface = IMG_LoadTyped_RW(io, 1, "PNG");
	// in case of an error (surface = nullptr) throw the error as an exception
	if (surface == nullptr) {
		auto sdl_error = IMG_GetError();
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load image %s! Error: %s\n", c_path_str, sdl_error);
		throw AMAssetLoadException(sdl_error);
	}

	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	// same as above error handling
	if (texture == nullptr) {
		auto sdl_error = SDL_GetError();
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load image %s! Error: %s\n", c_path_str, sdl_error);
		throw AMAssetLoadException(sdl_error);
	}

	UITexture::UIProperties ui_props;

	// scaling is represented by 2 bytes in a row
	char high_scale_byte = signature_data[6];
	char low_scale_byte = signature_data[7];

	// example: 17 DE -> high_byte = 0x17; low_byte = 0xDE;
	// high_byte * 0x100 = 0x1700
	// scaling = (0x1700 + low_byte) / 100 = 0x17DE / 100 = 6110 / 100 = 61.10
	ui_props.scaling = (static_cast<float>(high_scale_byte) * 0x100 + static_cast<float>(low_scale_byte)) / 100;

	// stretching settings are represented by a single byte, 
	// where the second bit (0b10) represents the X stretch
	// and the first bit (0b01) represents the Y stretch
	char stretch_byte = signature_data[8];
	bitset<8> stretch_bits(stretch_byte);
	ui_props.stretch_x = stretch_bits[1];
	ui_props.stretch_y = stretch_bits[0];

	// cutting margins are represented by 2 bytes in a row in the following order:
	// left right top bottom
	ui_props.left =
		static_cast<uint>(signature_data[9]) * 0x100 +
		static_cast<uint>(signature_data[10]);

	ui_props.right =
		static_cast<uint>(signature_data[11]) * 0x100 +
		static_cast<uint>(signature_data[12]);

	ui_props.top =
		static_cast<uint>(signature_data[13]) * 0x100 +
		static_cast<uint>(signature_data[14]);

	ui_props.bottom =
		static_cast<uint>(signature_data[15]) * 0x100 +
		static_cast<uint>(signature_data[16]);

	// encapsulate raw texture pointer, it's width and height in a Texture object
	UITexture t_data(surface->w, surface->h, texture, ui_props);

	// after the texture is loaded, the surface is no longer needed, so it is freed
	SDL_FreeSurface(surface);

	// we don't need signature_data, free it
	free(signature_data);

	ui_textures.insert({ id, t_data });
}

void AssetManager::unload_ui_texture(const string& id) {
	// if the texture wasn't found
	if (ui_textures.find(id) == ui_textures.end()) return;

	// we don't need the texture anymore, destroy it
	SDL_DestroyTexture(ui_textures.at(id).get_raw());

	// erase the record of said texture
	ui_textures.erase(id);
}

void AssetManager::load_level(const string& id, const string& path, SDL_Renderer* renderer) {
	
}

void AssetManager::unload_level(const string& id) {
	// if the texture wasn't found
	if (levels.find(id) == levels.end()) return;

	// we don't need the texture anymore, destroy it
	//SDL_DestroyTexture(levels.at(id).get_raw());

	// erase the record of said texture
	levels.erase(id);
}

void AssetManager::load_font(const string& id, const string& path, int font_size) {
	// the asset is already loaded, there's no need to load it again
	if (fonts.find(id) != fonts.end()) return;

	// construct the path string
	auto constructed_path = string(prefix) + path;
	auto c_path_str = constructed_path.c_str();

	try {
		Font font(c_path_str, font_size);
		fonts.insert({ id, font });
	}
	catch (FontCreationException e) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load font %s! Error: %s\n", c_path_str, e.what());
		throw AMAssetLoadException(e.what());
	}
}

void AssetManager::unload_font(const string& id) {
	if (fonts.find(id) == fonts.end()) return;

	fonts.erase(id);
}
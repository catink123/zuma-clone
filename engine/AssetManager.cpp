#include "../engine/AssetManager.h"

AMAssetLoadException::AMAssetLoadException(const char* msg) : msg(msg) {}
const char* AMAssetLoadException::what() { return msg; }

AssetManager::AssetManager() {}

AssetManager::~AssetManager() {
	// get all raw texture pointer and destroy the textures
	for (auto pair : textures)
		pair.second.destroy();

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

LevelData& AssetManager::get_level_data(const string& id) {
	if (levels.find(id) == levels.end())
		throw AMAssetNotRegisteredException();

	return levels.at(id);
}

Font& AssetManager::get_font(const string& id) {
	if (fonts.find(id) == fonts.end())
		throw AMAssetNotRegisteredException();

	return fonts.at(id);
}

bool AssetManager::is_signature_valid(const unsigned char* signature_data) {
	return signature_data[0] == 'C' && signature_data[1] == 'A' && signature_data[2] == 'A' && signature_data[3] == 'S' && signature_data[4] == 'S';
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
	unsigned char* signature_data = (unsigned char*)malloc(6 * sizeof(unsigned char));

	// if couldn't allocate, throw an error
	if (!signature_data) throw runtime_error("couldn't create allocate signature_data!");

	// if couldn't read first 6 bytes, log and throw an error
	if (SDL_RWread(io, signature_data, 6 * sizeof(unsigned char), 1) <= 0) {
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

float AssetManager::convert_float_type(unsigned char* data) {
	return static_cast<uint>(data[0]) * 0x100 + static_cast<uint>(data[1]);
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
	unsigned char* signature_data = (unsigned char*)malloc(17 * sizeof(unsigned char));

	// if couldn't allocate, throw an error
	if (!signature_data) throw runtime_error("couldn't create allocate signature_data!");

	// if couldn't read first 17 bytes, log and throw an error
	if (SDL_RWread(io, signature_data, 17 * sizeof(unsigned char), 1) <= 0) {
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
	unsigned char high_scale_byte = signature_data[6];
	unsigned char low_scale_byte = signature_data[7];

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

	ui_props.left = convert_float_type(signature_data + 9);
	ui_props.right = convert_float_type(signature_data + 11);
	ui_props.top = convert_float_type(signature_data + 13);
	ui_props.bottom = convert_float_type(signature_data + 15);

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

uint AssetManager::convert_uint_type(unsigned char* data) {
	unsigned char first_byte = data[0];
	unsigned char second_byte = data[1];
	return static_cast<uint>(first_byte) * 0x100 + static_cast<uint>(second_byte);
}

void AssetManager::load_level_data(const string& id, const string& path, SDL_Renderer* renderer) {
	// the asset is already loaded, there's no need to load it again
	if (levels.find(id) != levels.end()) return;

	// construct the path string
	auto constructed_path = string(prefix) + path;
	auto c_path_str = constructed_path.c_str();

	// open file for reading in binary form
	SDL_RWops* io = SDL_RWFromFile(c_path_str, "rb");
	// allocate a C string for signature data
	unsigned char* signature_data = (unsigned char*)malloc(13 * sizeof(unsigned char));

	// if couldn't allocate, throw an error
	if (!signature_data) throw runtime_error("couldn't allocate signature_data!");

	// if couldn't read first 13 bytes, log and throw an error
	if (SDL_RWread(io, signature_data, 13 * sizeof(unsigned char), 1) <= 0) {
		auto sdl_error = SDL_GetError();
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load level data %s! Error: %s\n", c_path_str, sdl_error);
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
	if (asset_type != AssetType::ATLevel) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Loading non-texture as texture!");
		throw AMAssetLoadException("invalid asset type");
	}

	LevelData l_data;

	l_data.player_position.x = convert_uint_type(signature_data + 6);
	l_data.player_position.y = convert_uint_type(signature_data + 8);

	l_data.track_speed_multiplier = convert_float_type(signature_data + 10) / 100;

	// construct C string for level track points
	uint point_count = static_cast<uint>(signature_data[12]);
	unsigned char* level_data = (unsigned char*)malloc(sizeof(unsigned char) * point_count * 4);

	// if couldn't allocate, throw an error
	if (!level_data) throw runtime_error("couldn't allocate level_data!");

	// if couldn't read level data bytes, log and throw an error
	if (SDL_RWread(io, level_data, point_count * 4 * sizeof(unsigned char), 1) <= 0) {
		auto sdl_error = SDL_GetError();
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load level data %s! Error: %s\n", c_path_str, sdl_error);
		throw AMAssetLoadException(sdl_error);
	}

	vector<vec2> track_points;

	for (int i = 0; i < point_count; i++) {
		unsigned char* x_data = level_data + i * 4;
		unsigned char* y_data = level_data + i * 4 + 2;
		track_points.push_back(vec2(
			static_cast<float>(convert_uint_type(x_data)),
			static_cast<float>(convert_uint_type(y_data))
		));
	}

	l_data.track_points = track_points;

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

	Texture texture_obj(surface->w, surface->h, texture);

	textures.insert({ id, move(texture_obj) });

	l_data.background = &get_texture(id);

	// after the texture is loaded, the surface is no longer needed, so it is freed
	SDL_FreeSurface(surface);

	// we don't need signature_data and level_data anymore, free them
	free(signature_data);
	free(level_data);

	levels.insert({ id, move(l_data) });
}

void AssetManager::unload_level_data(const string& id) {
	// if the texture wasn't found
	if (levels.find(id) == levels.end()) return;

	// we don't need the texture anymore, unload it
	unload_texture(id);

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
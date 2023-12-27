#include "../engine/AssetManager.h"

AMAssetLoadException::AMAssetLoadException(const char* msg) : msg(msg) {}
const char* AMAssetLoadException::what() { return msg; }

AssetManager::AssetManager() {}

AssetManager::~AssetManager() {
	for (auto& pair : textures)
		pair.second.destroy();

	for (auto& pair : ui_textures)
		pair.second.destroy();

	for (auto& pair : fonts)
		pair.second.destroy();

	for (auto& pair : audio)
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

Audio& AssetManager::get_audio(const string& id) {
	if (audio.find(id) == audio.end())
		throw AMAssetNotRegisteredException();

	return audio.at(id);
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
	SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "AssetManager: Loading a texture with id '%s' on path '%s'...\n", id.c_str(), c_path_str);

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
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Signature of loaded file is invalid!\n");
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

	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

	// encapsulate raw texture pointer, it's width and height in a Texture object
	Texture t_data(surface->w, surface->h, texture);

	// after the texture is loaded, the surface is no longer needed, so it is freed
	SDL_FreeSurface(surface);

	textures.insert({ id, t_data });
}

void AssetManager::unload_texture(const string& id) {
	// if the texture wasn't found
	if (textures.find(id) == textures.end()) return;
	SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "AssetManager: Unloading a texture with id '%s'...\n", id.c_str());

	// we don't need the texture anymore, destroy it
	SDL_DestroyTexture(textures.at(id).get_raw());

	// erase the record of said texture
	textures.erase(id);
}

float AssetManager::convert_float_type(unsigned char* data) {
	return static_cast<float>(static_cast<uint>(data[0]) * 0x100 + static_cast<uint>(data[1]));
}

void AssetManager::load_ui_texture(const string& id, const string& path, SDL_Renderer* renderer) {
	// the asset is already loaded, there's no need to load it again
	if (ui_textures.find(id) != ui_textures.end()) return;

	// construct the path string
	auto constructed_path = string(prefix) + path;
	auto c_path_str = constructed_path.c_str();
	SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "AssetManager: Loading a UI texture with id '%s' on path '%s'...\n", id.c_str(), c_path_str);

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

	ui_props.left = static_cast<uint>(convert_float_type(signature_data + 9));
	ui_props.right = static_cast<uint>(convert_float_type(signature_data + 11));
	ui_props.top = static_cast<uint>(convert_float_type(signature_data + 13));
	ui_props.bottom = static_cast<uint>(convert_float_type(signature_data + 15));

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
	SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "AssetManager: Unloading a UI texture with id '%s'...\n", id.c_str());

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

void AssetManager::load_level_data(const string& id, const path& asset_path, SDL_Renderer* renderer) {
	// the asset is already loaded, there's no need to load it again
	if (levels.find(id) != levels.end()) return;

	// construct the path string
	auto path_str = asset_path.string();
	auto c_path_str = path_str.c_str();

	log_verbose("AssetManager: Loading a level data with id '%s' on path '%s'...\n", id.c_str(), c_path_str);

	// open file for reading in binary form
	SDL_RWops* io = SDL_RWFromFile(c_path_str, "rb");

	if (!io) {
		auto sdl_error = SDL_GetError();
		log_error("AssetManager: Couldn't load level data on path '%s'! Error: %s.\n", c_path_str, sdl_error);
		throw AMAssetLoadException(sdl_error);
		return;
	}

	LevelData level_data;

	auto level_doc_size = SDL_RWsize(io);
	void* level_doc_bits = malloc(level_doc_size * sizeof(Sint64));

	SDL_RWread(io, level_doc_bits, level_doc_size, 1);

	pugi::xml_document level_doc;
	pugi::xml_parse_result parse_res = level_doc.load_buffer(level_doc_bits, level_doc_size);

	if (!level_doc.child("level")) {
		log_error("AssetManager: Invalid level data document on path '%s'!", c_path_str);
		return;
	}

	pugi::xpath_node_set xp_points = level_doc.select_nodes("/level/point");
	for (pugi::xpath_node xp_point : xp_points) {
		float x = xp_point.node().attribute("x").as_float();
		float y = xp_point.node().attribute("y").as_float();
		level_data.track_points.push_back(vec2(x, y));
	}

	level_data.name = level_doc.select_node("/level/name").node().text().as_string();

	string bg_path_text = level_doc.select_node("/level/background/@src").attribute().as_string();
	path bg_path = asset_path.parent_path();
	bg_path /= bg_path_text;

	string bg_path_str = bg_path.string();

	SDL_Texture* texture = IMG_LoadTexture(renderer, bg_path_str.c_str());
	if (!texture) {
		auto sdl_error = IMG_GetError();
		log_error("AssetManager: Couldn't load image from path '%s'! Error: %s\n", bg_path_str.c_str(), sdl_error);
		throw AMAssetLoadException(sdl_error);
	}

	int texture_width = 0;
	int texture_height = 0;

	SDL_QueryTexture(texture, nullptr, nullptr, &texture_width, &texture_height);

	Texture texture_obj(static_cast<ushort>(texture_width), static_cast<ushort>(texture_height), texture);

	textures.insert({ id, move(texture_obj) });

	level_data.background = &get_texture(id);

	auto plpos_node = level_doc.select_node("/level/player-position").node();
	level_data.player_position.x = plpos_node.attribute("x").as_float();
	level_data.player_position.y = plpos_node.attribute("y").as_float();

	level_data.track_ball_count = level_doc.select_node("/level/ball-count").node().text().as_uint();
	level_data.track_speed_multiplier = level_doc.select_node("/level/speed-multiplier").node().text().as_float();

	levels.insert({ id, move(level_data) });
}

void AssetManager::unload_level_data(const string& id) {
	// if the texture wasn't found
	if (levels.find(id) == levels.end()) return;
	SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "AssetManager: Unloading level data with id '%s'...\n", id.c_str());

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
	SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "AssetManager: Loading a font with id '%s' on path '%s'...\n", id.c_str(), c_path_str);

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
	SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "AssetManager: Unloading a font with id '%s'...\n", id.c_str());

	fonts.erase(id);
}

void AssetManager::load_audio(const string& id, const string& path, AudioType audio_type) {
	// the asset is already loaded, there's no need to load it again
	if (audio.find(id) != audio.end()) return;

	// construct the path string
	auto constructed_path = string(prefix) + path;
	auto c_path_str = constructed_path.c_str();
	SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "AssetManager: Loading audio with id '%s' on path '%s'...\n", id.c_str(), c_path_str);

	Mix_Chunk* chunk = nullptr;
	Mix_Music* music = nullptr;
	if (audio_type == Sound)
		chunk = Mix_LoadWAV(c_path_str);
	else
		music = Mix_LoadMUS(c_path_str);

	if (chunk == nullptr && music == nullptr) {
		auto sdl_error = Mix_GetError();
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load audio %s! Error: %s\n", c_path_str, sdl_error);
		throw AMAssetLoadException(sdl_error);
	}

	if (audio_type == Sound) {
		Audio audio_obj(chunk);
		audio.insert({ id, move(audio_obj) });
	}
	else {
		Audio audio_obj(music);
		audio.insert({ id, move(audio_obj) });
	}
}

void AssetManager::unload_audio(const string& id) {
	if (audio.find(id) == audio.end()) return;
	SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "AssetManager: Unloading audio with id '%s'...\n", id.c_str());

	audio.at(id).destroy();

	audio.erase(id);
}

void AssetManager::load_all_levels(SDL_Renderer* renderer) {
	filesystem::directory_iterator level_dir(string(prefix) + "/assets/levels");
	for (const auto& entry : level_dir) {
		if (entry.is_regular_file() && entry.path().extension() == ".xml") {
			load_level_data(entry.path().stem().string(), entry.path().string(), renderer);
		}
	}
}

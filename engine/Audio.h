#pragma once
#include <SDL_mixer.h>
#include <variant>

enum AudioType {
	Sound, Music
};

class Audio {
	Mix_Chunk* chunk = nullptr;
	Mix_Music* music = nullptr;
public:
	Audio(std::variant<Mix_Chunk*, Mix_Music*> audio_data) {
		if (holds_alternative<Mix_Chunk*>(audio_data))
			chunk = get<Mix_Chunk*>(audio_data);
		else
			music = get<Mix_Music*>(audio_data);
	}

	Mix_Chunk* get_sound_raw() { return chunk; }
	Mix_Music* get_music_raw() { return music; }
	AudioType get_type() {
		if (music == nullptr)
			return Sound;
		else
			return Music;
	}

	void destroy() {
		if (chunk)
			Mix_FreeChunk(chunk);
	}
};

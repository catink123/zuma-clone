#pragma once
#include <SDL.h>
#include <SDL_mixer.h>
#include <stdexcept>
#include "Audio.h"

struct SMPlayException : std::runtime_error {
	SMPlayException(const char* msg) : runtime_error(msg) {}
};

struct SoundManager {
	static constexpr float MUSIC_VOLUME = 0.5F;

	static void play_sound(Audio& audio) {
		if (audio.get_type() == Music) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SoundManager: error: tried playing music as a sound.\n");
			throw SMPlayException("music as sound play attempt");
		}

		Mix_PlayChannel(-1, audio.get_sound_raw(), 0);
	}

	static void set_music(Audio& audio) {
		if (audio.get_type() == Sound) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SoundManager: error: tried playing sound as a sound.\n");
			throw SMPlayException("sound as music play attempt");
		}

		Mix_PlayMusic(audio.get_music_raw(), 0);
	}

	static void pause_music() {
		Mix_PauseMusic();
	}

	static void resume_music() {
		Mix_ResumeMusic();
	}

	static void stop_music() {
		Mix_HaltMusic();
	}

	static void set_music_volume(const float& volume) {
		Mix_VolumeMusic(static_cast<int>(volume * MIX_MAX_VOLUME));
	}

	static float get_music_volume() {
		return static_cast<float>(Mix_VolumeMusic(-1)) / static_cast<float>(MIX_MAX_VOLUME);
	}

	static void set_volume(const float& volume) {
		Mix_MasterVolume(static_cast<int>(volume * static_cast<float>(MIX_MAX_VOLUME)));
		set_music_volume(volume * MUSIC_VOLUME);
	}

	static float get_volume() {
		return static_cast<float>(Mix_MasterVolume(-1)) / static_cast<float>(MIX_MAX_VOLUME);
	}
};
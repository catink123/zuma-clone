#pragma once
#include <SDL.h>
#undef main
#include <SDL_image.h>
#include "Common.h"

class Texture {
	ushort w;
	ushort h;
	SDL_Texture* texture;

public:
	Texture(ushort w, ushort h, SDL_Texture* texture) : w(w), h(h), texture(texture) {}
	Texture() : w(0), h(0), texture(nullptr) {}

	SDL_Rect get_rect(int x = 0, int y = 0, float scale = 1) const;
	ushort get_width() const;
	ushort get_height() const;
	SDL_Texture* get_raw() const;
};


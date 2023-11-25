#pragma once
#include <SDL.h>
#undef main
#include <SDL_image.h>
#include "common.h"

// A class that encapsulates the raw SDL_Texture and it's surface's width and height
class Texture {
	ushort w;
	ushort h;
	SDL_Texture* texture;

public:
	Texture(const ushort& w, const ushort& h, SDL_Texture* texture) : w(w), h(h), texture(texture) {}
	Texture() : w(0), h(0), texture(nullptr) {}

	// get a standard rectangle for drawing the texture using draw() method
	SDL_FRect get_rect(const float& x = 0, const float& y = 0, const float& scale = 1) const;
	ushort get_width() const;
	ushort get_height() const;
	SDL_Texture* get_raw() const;

	bool operator==(const Texture& other) const;
	bool operator!=(const Texture& other) const;
};


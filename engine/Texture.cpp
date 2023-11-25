#include "../engine/Texture.h"
#include <cmath>

SDL_FRect Texture::get_rect(const float& x, const float& y, const float& scale) const {
	SDL_FRect rect;
	rect.w = this->w * scale;
	rect.h = this->h * scale;
	rect.x = x;
	rect.y = y;

	return rect;
}

ushort Texture::get_width() const { return w; }
ushort Texture::get_height() const { return h; }

SDL_Texture* Texture::get_raw() const { return texture; }

bool Texture::operator==(const Texture& other) const {
	return texture == other.get_raw();
}

bool Texture::operator!=(const Texture& other) const {
	return texture != other.get_raw();
}

#include "Texture.h"
#include <cmath>

SDL_Rect Texture::get_rect(int x, int y, float scale) const {
	SDL_Rect rect;
	rect.w = static_cast<int>(this->w * scale);
	rect.h = static_cast<int>(this->h * scale);
	rect.x = x;
	rect.y = y;

	return rect;
}

ushort Texture::get_width() const { return w; }
ushort Texture::get_height() const { return h; }

SDL_Texture* Texture::get_raw() const { return texture; }

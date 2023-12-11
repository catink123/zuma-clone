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
void Texture::destroy() {
	if (texture == nullptr)
		return;

	SDL_DestroyTexture(texture);
	texture = nullptr;
	w = 0;
	h = 0;
}

bool Texture::operator==(const Texture& other) const {
	return texture == other.get_raw();
}

bool Texture::operator!=(const Texture& other) const {
	return texture != other.get_raw();
}

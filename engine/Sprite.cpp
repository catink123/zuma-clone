#include "../engine/Sprite.h"

void Sprite::change_texture(Texture* new_texture) {
	texture = new_texture;
}

vec2 Sprite::get_size() const {
	if (display_size != nullptr)
		return *display_size;
	else if (clip_rect != nullptr)
		return vec2(
			clip_rect->w * scale.x,
			clip_rect->h * scale.y
		);
	else
		return vec2(
			texture->get_width() * scale.x,
			texture->get_height() * scale.y
		);
}

const Texture* Sprite::get_texture() const {
	return texture;
}

void Sprite::draw(SDL_Renderer* renderer, const RendererState& renderer_state) const {
	auto output_rect = 
		texture->get_rect(
			static_cast<int>(position.x), 
			static_cast<int>(position.y)
		);

	if (display_size != nullptr) {
		output_rect.w = static_cast<int>(display_size->x);
		output_rect.h = static_cast<int>(display_size->y);
	}
	else if (clip_rect != nullptr) {
		output_rect.w = clip_rect->w;
		output_rect.h = clip_rect->h;
	}


	// apply normal scaling
	output_rect.w = static_cast<int>(output_rect.w * scale.x);
	output_rect.h = static_cast<int>(output_rect.h * scale.y);

	// apply alignment
	switch (horizontal_alignment) {
	case Left:
		output_rect.x = static_cast<int>(position.x);
		break;
	case Center:
		output_rect.x = static_cast<int>(position.x - output_rect.w / 2.0F);
		break;
	case Right:
		output_rect.x = static_cast<int>(position.x - output_rect.w);
		break;
	}
	switch (vertical_alignment) {
	case Top:
		output_rect.y = static_cast<int>(position.y);
		break;
	case Middle:
		output_rect.y = static_cast<int>(position.y - output_rect.h / 2.0F);
		break;
	case Bottom:
		output_rect.y = static_cast<int>(position.y - output_rect.h);
		break;
	}

	// apply window scaling
	float scaling = renderer_state.scaling;

	output_rect.x = static_cast<int>(output_rect.x * scaling);
	output_rect.y = static_cast<int>(output_rect.y * scaling);
	output_rect.w = static_cast<int>(output_rect.w * scaling);
	output_rect.h = static_cast<int>(output_rect.h * scaling);

	SDL_RenderCopyEx(renderer, texture->get_raw(), clip_rect, &output_rect, rotation, nullptr, SDL_FLIP_NONE);
}

void Sprite::set_clip_rect(SDL_Rect* rect) {
	clip_rect = rect;
}

void Sprite::set_display_size(const vec2& size) {
	vec2* s = new vec2(size);
	display_size = s;
}
#include "../engine/Sprite.h"

void Sprite::change_texture(const Texture* new_texture) {
	texture = new_texture;
}

vec2 Sprite::get_size() const {
	if (display_size != nullopt)
		return *display_size;
	else if (clip_rect != nullopt)
		return vec2(
			clip_rect->w * global_transform.scale.x,
			clip_rect->h * global_transform.scale.y
		);
	else
		return vec2(
			texture->get_width() * global_transform.scale.x,
			texture->get_height() * global_transform.scale.y
		);
}

const Texture* Sprite::get_texture() const {
	return texture;
}

void Sprite::draw(SDL_Renderer* renderer, const RendererState& renderer_state) const {
	Transform resulting_transform = get_calculated_transform();

	auto output_rect = 
		texture->get_rect(
			resulting_transform.position.x, 
			resulting_transform.position.y
		);

	if (display_size != nullopt) {
		output_rect.w = display_size->x;
		output_rect.h = display_size->y;
	}
	else if (clip_rect != nullopt) {
		output_rect.w = static_cast<float>(clip_rect->w);
		output_rect.h = static_cast<float>(clip_rect->h);
	}


	// apply normal scaling
	output_rect.w = output_rect.w * resulting_transform.scale.x;
	output_rect.h = output_rect.h * resulting_transform.scale.y;

	// apply alignment
	switch (horizontal_alignment) {
	case Left:
		output_rect.x = resulting_transform.position.x;
		break;
	case Center:
		output_rect.x = resulting_transform.position.x - output_rect.w / 2.0F;
		break;
	case Right:
		output_rect.x = resulting_transform.position.x - output_rect.w;
		break;
	}
	switch (vertical_alignment) {
	case Top:
		output_rect.y = resulting_transform.position.y;
		break;
	case Middle:
		output_rect.y = resulting_transform.position.y - output_rect.h / 2.0F;
		break;
	case Bottom:
		output_rect.y = resulting_transform.position.y - output_rect.h;
		break;
	}

	// apply window scaling
	float scaling = renderer_state.scaling;

	output_rect.x = output_rect.x * scaling;
	output_rect.y = output_rect.y * scaling;
	output_rect.w = output_rect.w * scaling;
	output_rect.h = output_rect.h * scaling;

	const SDL_Rect* cr = nullptr;
	if (clip_rect)
		cr = &clip_rect.value();

	SDL_RenderCopyExF(renderer, texture->get_raw(), cr, &output_rect, resulting_transform.rotation, nullptr, SDL_FLIP_NONE);
}

void Sprite::set_display_size(const vec2& size) {
	vec2 s(size);
	display_size = s;
}

Transform Sprite::get_calculated_transform() const {
	Transform resulting_transform;

	if (origin_transform != nullptr)
		resulting_transform = *origin_transform + resulting_transform;

	resulting_transform = resulting_transform + global_transform + local_transform;

	return resulting_transform;
}

void Sprite::apply_origin_transform() {
	if (origin_transform != nullptr)
		global_transform = *origin_transform + global_transform;
	origin_transform = nullptr;
}

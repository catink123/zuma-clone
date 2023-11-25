#pragma once
#include "basics.h"
#include "Texture.h"
#include <optional>
#include "../game/GameState.h"

enum VerticalAlignment {
	Top, Middle, Bottom
};

enum HorizontalAlignment {
	Left, Center, Right
};

class Sprite : public Drawable {
	const Texture* texture;
	optional<vec2> display_size;

public:
	optional<SDL_Rect> clip_rect;

	Transform global_transform;
	Transform local_transform;
	Transform* origin_transform = nullptr;

	VerticalAlignment vertical_alignment = Top;
	HorizontalAlignment horizontal_alignment = Left;

	Sprite(
		const Texture* texture, 
		vec2 position = vec2(), 
		vec2 scale = vec2(1, 1), 
		float rotation = 0,
		optional<SDL_Rect> clip_rect = nullopt,
		optional<vec2> display_size = nullopt
	) :
		global_transform(position, scale, rotation), 
		texture(texture), 
		clip_rect(clip_rect),
		display_size(display_size)
	{}

	void change_texture(const Texture* new_texture);
	vec2 get_size() const;
	const Texture* get_texture() const;
	void set_display_size(const vec2& size);
	virtual void draw(SDL_Renderer* renderer, const RendererState& renderer_state) const override;

	Transform get_calculated_transform() const;
	void apply_origin_transform();
};

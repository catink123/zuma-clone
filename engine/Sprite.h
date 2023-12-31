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

using namespace std;

class Sprite : public Drawable {
protected:
	Texture* texture;
	optional<vec2> display_size;

public:
	optional<SDL_Rect> clip_rect;

	Transform global_transform;
	Transform local_transform;
	Transform* origin_transform = nullptr;

	VerticalAlignment vertical_alignment = Top;
	HorizontalAlignment horizontal_alignment = Left;

	float opacity = 1;

	Sprite(
		Texture* texture, 
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

	void change_texture(Texture* new_texture);
	vec2 get_size() const;
	Texture* get_texture() const;
	void set_display_size(const vec2& size);
	virtual void draw(SDL_Renderer* renderer, const RendererState& renderer_state) const override;

	// Transform methods

	// returns a sum of global_transform, local_transform and origin_transform (if exists)
	Transform get_calculated_transform() const;
	// applies (sums up) visual origin_transform to global_transform and removes origin_transform
	void apply_origin_transform();
};

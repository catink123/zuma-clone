#pragma once
#include "basics.h"
#include "Texture.h"
#include "../game/GameState.h"

enum VerticalAlignment {
	Top, Middle, Bottom
};

enum HorizontalAlignment {
	Left, Center, Right
};

class Sprite : public Drawable, public Transformable {
	Texture* texture;
	SDL_Rect* clip_rect;
	vec2* display_size;

public:
	VerticalAlignment vertical_alignment = Top;
	HorizontalAlignment horizontal_alignment = Left;

	Sprite(
		Texture* texture, 
		vec2 position = vec2(), 
		vec2 scale = vec2(1, 1), 
		float rotation = 0,
		SDL_Rect* clip_rect = nullptr,
		vec2* display_size = nullptr
	) :
		Transformable(position, scale, rotation), 
		texture(texture), 
		clip_rect(clip_rect),
		display_size(display_size)
	{}

	void change_texture(Texture* new_texture);
	vec2 get_size() const;
	const Texture* get_texture() const;
	void set_clip_rect(SDL_Rect* clip_rect);
	void set_display_size(const vec2& size);
	virtual void draw(SDL_Renderer* renderer, const RendererState& renderer_state) const;
};

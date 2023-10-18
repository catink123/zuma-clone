#pragma once
#include <SDL.h>
#include "common.h"
#include "../game/GameState.h"
#include "Animation.h"

class Transform {
public:
	vec2 position;
	vec2 scale;
	float rotation = 0;

	Transform(
		vec2 position = vec2(0, 0), 
		vec2 scale = vec2(1, 1), 
		float rotation = 0
	) : position(position), scale(scale), rotation(rotation) {}

	Transform(
		float x, 
		float y, 
		float x_scale = 1, 
		float y_scale = 1, 
		float rotation = 0
	) : position(vec2(x, y)), scale(vec2(x_scale, y_scale)), rotation(rotation) {}

	Transform operator+(const Transform& other) const;
	Transform operator-(const Transform& other) const;
};

class Drawable {
public: 
	virtual void draw(SDL_Renderer* renderer, const RendererState& renderer_state) = 0;
};

class Animatable {
public:
	virtual void update(const float& delta) = 0;
};

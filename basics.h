#pragma once
#include <SDL.h>
#include "common.h"
#include "GameState.h"

class Transformable {
public:
	vec2 position;
	vec2 scale;
	float rotation;

	Transformable(
		vec2 position = vec2(0, 0), 
		vec2 scale = vec2(1, 1), 
		float rotation = 0
	) : position(position), scale(scale), rotation(rotation) {}

	Transformable(
		float x = 0, 
		float y = 0, 
		float x_scale = 1, 
		float y_scale = 1, 
		float rotation = 0
	) : position(vec2(x, y)), scale(vec2(x_scale, y_scale)), rotation(rotation) {}

	Transformable() : rotation(0) {}
};

class Drawable {
public: 
	virtual void draw(SDL_Renderer* renderer, const RendererState& renderer_state) = 0;
};

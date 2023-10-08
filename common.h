#pragma once
#include <utility>
#include <SDL.h>

typedef unsigned short ushort;
typedef unsigned int uint;

typedef struct vec2 {
	float x;
	float y;

	vec2() : x(0), y(0) {}
	vec2(float x, float y) : x(x), y(y) {}
	vec2(std::pair<float, float> params) : x(params.first), y(params.second) {}
	vec2(const vec2& other) : x(other.x), y(other.y) {}
} vec2;

static const SDL_Rect DEFAULT_RECT;

float rad_to_deg(float radians);

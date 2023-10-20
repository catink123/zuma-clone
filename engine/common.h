#pragma once
#include <utility>
#include <SDL.h>

typedef uint16_t ushort;
typedef uint32_t uint;

typedef struct vec2 {
	float x;
	float y;

	vec2() : x(0), y(0) {}
	vec2(float x, float y) : x(x), y(y) {}
	vec2(const vec2& start_point, const vec2& end_point) : x(end_point.x - start_point.x), y(end_point.y - start_point.y) {}
	vec2(std::pair<float, float> params) : x(params.first), y(params.second) {}
	vec2(const vec2& other) : x(other.x), y(other.y) {}

	vec2 operator+(const vec2& other) const;
	vec2 operator-(const vec2& other) const;
	vec2 operator*(const vec2& other) const;
	vec2 operator*(const float& amount) const;
	vec2 operator/(const vec2& other) const;
	vec2 operator/(const float& amount) const;
	void operator=(const float& amount);

	float len() const;
	static float dot(const vec2& first, const vec2& second);
} vec2;

static const SDL_Rect DEFAULT_RECT;

float rad_to_deg(const float& radians);
float deg_to_rad(const float& degrees);

// convert given angle to a standard 0-360 format
float normalize_angle(const float& angle);

class Collision {
public:
	static bool is_circle_on_line(
		const vec2& line_start_point,
		const vec2& line_end_point,
		const vec2& circle_center,
		const float& radius
	);
	static bool is_point_on_line(
		const vec2& line_start_point, 
		const vec2& line_end_point, 
		const vec2& point, 
		const float& error = 0.1
	);
	static bool is_point_in_circle(
		const vec2& circle_center, 
		const float& circle_radius, 
		const vec2& point
	);
	static bool is_circle_on_circle(
		const vec2& first_circle, 
		const float& first_circle_radius, 
		const vec2& second_circle, 
		const float& second_circle_radius
	);
};

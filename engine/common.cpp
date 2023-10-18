#include "../engine/common.h"

vec2 vec2::operator+(const vec2& other) const { return vec2(this->x + other.x, this->y + other.y); }
vec2 vec2::operator-(const vec2& other) const { return vec2(this->x - other.x, this->y - other.y); }
vec2 vec2::operator*(const vec2& other) const { return vec2(this->x * other.x, this->y * other.y); }
vec2 vec2::operator*(const float& amount) const { return vec2(this->x * amount, this->y * amount); }
vec2 vec2::operator/(const vec2& other) const { return vec2(this->x / other.x, this->y / other.y); }
vec2 vec2::operator/(const float& amount) const { return vec2(this->x / amount, this->y / amount); }

float rad_to_deg(const float& radians) {
	return radians * (180 / static_cast<float>(M_PI));
}

float deg_to_rad(const float& degrees) {
	return degrees * (static_cast<float>(M_PI) / 180);
}

float normalize_angle(const float& angle) {
	int remainder = static_cast<int>(angle) % 360;
	float resulting_angle = static_cast<float>(remainder);
	if (remainder < 0) resulting_angle += 360;

	return resulting_angle;
}
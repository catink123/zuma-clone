#include "../engine/common.h"

float rad_to_deg(const float& radians) {
	return radians * (180 / static_cast<float>(M_PI));
}

float normalize_angle(const float& angle) {
	int remainder = static_cast<int>(angle) % 360;
	float resulting_angle = remainder;
	if (remainder < 0) resulting_angle += 360;

	return resulting_angle;
}
#include "common.h"

float rad_to_deg(float radians) {
	return radians * (180 / static_cast<float>(M_PI));
}
#include "Balls.h"

void Ball::draw(SDL_Renderer* renderer) {
	float step = 360.0 / BALL_ROTATION_FRAME_COUNT;
	uint ball_frame = static_cast<uint>(floor(ball_angle / step));
}

float Ball::get_ball_angle() const { return ball_angle; }
void Ball::set_ball_angle(float angle) {
	float remainder = (int)angle % 360;
	if (remainder < 0)	ball_angle = 360 + remainder;
	else				ball_angle = remainder;
}
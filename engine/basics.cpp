#include "../engine/basics.h"

Transform Transform::operator+(const Transform& other) const {
	// the new position will rotate on the origin (current position) by current rotation value
	float angle_rad = deg_to_rad(rotation);

	// distance between points is the length of vector AB accounted for scale, 
	// where A - this->position, B - other.position
	float len_from_origin = hypotf(other.position.x * this->scale.x, other.position.y * this->scale.y);
	if (len_from_origin == 0) len_from_origin = 0.00001F; // prevent division by zero in the cosine calculation

	float origin_angle = acosf(other.position.x / len_from_origin);
	// if the other point is over this point, we need to negate the angle between the origin 
	// and the global X axis, because acos in <cmath> doesn't produce negative numbers
	if (other.position.y < 0) origin_angle *= -1;

	float resulting_angle = deg_to_rad(this->rotation) + origin_angle;

	// the new position is this->position + other.position rotated about this->position by resulting_angle
	vec2 new_position(
		this->position.x + len_from_origin * cosf(resulting_angle),
		this->position.y + len_from_origin * sinf(resulting_angle)
	);

	return Transform(
		new_position, 
		this->scale * other.scale, 
		this->rotation + other.rotation
	);
}

bool Timer::is_done() const { return current_time >= duration; }

void Timer::reset(bool save_overrun) {
	if (save_overrun && is_done())
		current_time -= duration;
	else
		current_time = 0;
}

void Timer::set_progress(const float& progress) {
	current_time = duration * progress;
}

void Timer::update(const float& delta, GameState&) {
	current_time += delta;
}

const float& Timer::get_current_time() const { return current_time; }

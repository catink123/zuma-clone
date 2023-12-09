#include "../engine/common.h"

vec2 vec2::operator+(const vec2& other) const { return vec2(this->x + other.x, this->y + other.y); }
vec2 vec2::operator-(const vec2& other) const { return vec2(this->x - other.x, this->y - other.y); }
vec2 vec2::operator*(const vec2& other) const { return vec2(this->x * other.x, this->y * other.y); }
vec2 vec2::operator*(const float& amount) const { return vec2(this->x * amount, this->y * amount); }
vec2 vec2::operator/(const vec2& other) const { return vec2(this->x / other.x, this->y / other.y); }
vec2 vec2::operator/(const float& amount) const { return vec2(this->x / amount, this->y / amount); }
void vec2::operator=(const float& amount) { this->x = amount; this->y = amount; }
void vec2::operator+=(const vec2& other) { this->x += other.x; this->y += other.y; }
float vec2::len() const { return hypotf(x, y); }

float vec2::dot(const vec2& first, const vec2& second) {
	return first.x * second.x + first.y * second.y;
}

float rad_to_deg(const float& radians) {
	return radians * (180 / static_cast<float>(M_PI));
}

float deg_to_rad(const float& degrees) {
	return degrees * (static_cast<float>(M_PI) / 180);
}

float normalize_angle(const float& angle) {
	float remainder = remainderf(angle, 360.0F);
	float resulting_angle = remainder;
	if (remainder < 0) resulting_angle += 360;

	return resulting_angle;
}

bool Collision::is_circle_on_line(
	const vec2& line_start_point,
	const vec2& line_end_point,
	const vec2& circle_center,
	const float& radius
) {
	bool start_on_circle = Collision::is_circle_on_circle(line_start_point, radius, circle_center, radius);
	bool end_on_circle = Collision::is_circle_on_circle(line_end_point, radius, circle_center, radius);
	if (start_on_circle || end_on_circle)
		return true;
	
	// closest point from circle on the segment
	vec2 closest_point = get_closest_point_on_line(line_start_point, line_end_point, circle_center);
	
	// if the closest point to segment is not on the segment, the given point is not on the segment
	if (!Collision::is_point_on_line(line_start_point, line_end_point, closest_point))
		return false;

	float point_to_closest_distance = (circle_center - closest_point).len();
	return point_to_closest_distance <= radius * 2;
}

vec2 Collision::get_closest_point_on_line(
	const vec2& line_start_point,
	const vec2& line_end_point,
	const vec2& point
) {
	vec2 line_vec(line_start_point, line_end_point);
	
	vec2 line_to_point_vec(line_start_point, point);
	float closest_point_length_fraction = vec2::dot(line_vec, line_to_point_vec) / powf(line_vec.len(), 2);
	
	// closest point from circle on the segment
	vec2 closest_point = line_start_point + (line_vec * closest_point_length_fraction);
	
	return closest_point;
}

bool Collision::is_point_on_line(
	const vec2& line_start_point, 
	const vec2& line_end_point, 
	const vec2& point, 
	const float& error
) {
	vec2 line_vec(line_start_point, line_end_point);
	float line_length = line_vec.len();

	float point_to_start_distance = vec2(line_start_point, point).len();
	float point_to_end_distance = vec2(line_end_point, point).len();

	return 
		point_to_start_distance + point_to_end_distance >= line_length - error
		&&
		point_to_start_distance + point_to_end_distance <= line_length + error;
}

bool Collision::is_point_in_circle(
	const vec2& circle_center,
	const float& circle_radius,
	const vec2& point
) {
	float point_to_circle_distance = vec2(circle_center, point).len();
	return point_to_circle_distance <= circle_radius;
}

bool Collision::is_circle_on_circle(
	const vec2& first_circle_center,
	const float& first_circle_radius,
	const vec2& second_circle_center,
	const float& second_circle_radius
) {
	float distance_between_circles = vec2(first_circle_center, second_circle_center).len();
	return distance_between_circles <= first_circle_radius + second_circle_radius;
}

float rand_float() {
	return static_cast<float>(rand() % 1000000) / 1000000;
}
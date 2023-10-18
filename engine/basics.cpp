#include "../engine/basics.h"

Transform Transform::operator+(const Transform& other) const {
	return Transform(
		this->position + other.position, 
		this->scale * other.scale, 
		this->rotation + other.rotation
	);
}

Transform Transform::operator-(const Transform& other) const {
	return Transform(
		this->position - other.position,
		this->scale / other.scale,
		this->rotation - other.rotation
	);
}
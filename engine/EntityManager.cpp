#include "EntityManager.h"

template <typename T>
vector<shared_ptr<T>> EntityManager::get_drawables_by_type() {
	vector<shared_ptr<T>> result;

	for (shared_ptr<Drawable> dr : drawables) {
		shared_ptr<T> cast = dynamic_pointer_cast<T>(dr);
		if (cast != nullptr)
			result.push_back(cast);
	}

	return result;
}

template <class T>
shared_ptr<T> EntityManager::get_drawable_by_name(const char* name) {
	if (drawable_map.find(name) == drawable_map.end()) throw EngineDrawableNonexistentException();
	return dynamic_pointer_cast<T>(drawable_map.at(name));
}

shared_ptr<Drawable> EntityManager::add_drawable(string id, shared_ptr<Drawable> drawable) {
	// if the drawable with given id already exists, don't add anything
	if (drawable_map.find(id) != drawable_map.end()) return nullptr;
	drawable_map.insert({ id, drawable });
	add_drawable_raw(drawable);

	return drawable;
}

void EntityManager::add_drawable_raw(shared_ptr<Drawable> drawable) {
	drawables.push_back(drawable);
}

void EntityManager::remove_drawable(string id) {
	if (drawable_map.find(id) == drawable_map.end()) return;
	drawable_map.erase(id);
}

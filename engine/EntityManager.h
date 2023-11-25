#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "basics.h"

using namespace std;

typedef unordered_map<string, shared_ptr<Drawable>> drawable_map;

class EntityManager {
	vector<shared_ptr<Drawable>> drawables;
	drawable_map drawable_map;

public:

	template <typename T>
	vector<shared_ptr<T>> get_drawables_by_type();

	template <typename T>
	shared_ptr<T> get_drawable_by_name(const char* name);

	shared_ptr<Drawable> add_drawable(string id, shared_ptr<Drawable> drawable);
	void remove_drawable(string id);

	void add_drawable_raw(shared_ptr<Drawable> drawable);
};

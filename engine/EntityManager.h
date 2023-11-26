#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "basics.h"

using namespace std;

typedef unordered_map<string, shared_ptr<Drawable>> entity_map;

// Manages all entities in the engine
struct EntityManager {
	// raw entity pointers container
	vector<shared_ptr<Drawable>> entities;
	// table of named pointers to entities
	entity_map entity_map;

	EntityManager() {}

	// filters the entities container by type inherited from Drawable
	template <typename T>
	vector<shared_ptr<T>> get_entities_by_type() const {
		vector<shared_ptr<T>> result;

		for (shared_ptr<Drawable> dr : entities) {
			shared_ptr<T> cast = dynamic_pointer_cast<T>(dr);
			if (cast != nullptr)
				result.push_back(cast);
		}

		return result;
	}

	// returns the entity from the entity table by name and type inherited from Drawable
	template <typename T>
	shared_ptr<T> get_entity_by_name(const char* name) const {
		if (entity_map.find(name) == entity_map.end()) throw EntityNonexistentException();
		return dynamic_pointer_cast<T>(entity_map.at(name));
	}

	// adds a new entity to the entities container and assigns it a name in the table
	template <typename T>
	shared_ptr<T> add_entity(string id, shared_ptr<T> entity) {
		// if the drawable with given id already exists, don't add anything
		if (entity_map.find(id) != entity_map.end()) return nullptr;
		entity_map.insert({ id, entity });
		add_entity_raw(entity);

		return entity;
	}

	void remove_entity(string id) {
		if (entity_map.find(id) == entity_map.end()) return;
		entity_map.erase(id);
	}

	// same as above "add", except the entity isn't added to the table
	template <typename T>
	shared_ptr<T> add_entity_raw(shared_ptr<T> drawable) {
		entities.push_back(drawable);
		return drawable;
	}
};

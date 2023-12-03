#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include "basics.h"

using namespace std;

class EntityNonexistentException : public logic_error {
public:
	EntityNonexistentException(const string& what_arg) : logic_error(what_arg) {}
};

typedef unordered_map<string, shared_ptr<Drawable>> entity_map;
typedef unordered_map<GameSection, vector<shared_ptr<Drawable>>> section_map;

// Manages all entities in the engine
struct EntityManager {
	// raw entity pointers container
	vector<shared_ptr<Drawable>> entities;
	// table of named pointers to entities
	entity_map entity_map;
	section_map section_map = {
		{ InLevel, {} },
		{ InMenu, {} },
		{ None, {} }
	};

	// entity pointers scheduled to delete in the next frame
	vector<Drawable*> entities_to_delete;

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

	// filters the entities by GameSection
	const vector<shared_ptr<Drawable>>& get_entities_by_section(GameSection section) const {
		return section_map.at(section);
	}

	// filters the entities by GameSection and type inherited from Drawable
	template <typename T>
	vector<shared_ptr<T>> get_entities_by_section_and_type(GameSection section) const {
		vector<shared_ptr<T>> result;

		for (shared_ptr<Drawable> dr : section_map.at(section)) {
			shared_ptr<T> cast = dynamic_pointer_cast<T>(dr);
			if (cast != nullptr)
				result.push_back(cast);
		}

		return result;
	}

	// returns the entity from the entity table by name and type inherited from Drawable
	template <typename T>
	shared_ptr<T> get_entity_by_name(const char* name) const {
		if (entity_map.find(name) == entity_map.end()) throw EntityNonexistentException(string("no entity named ") + name);
		return dynamic_pointer_cast<T>(entity_map.at(name));
	}

	// adds a new entity to the entities container and assigns it a name in the table
	template <typename T>
	shared_ptr<T> add_entity(string id, shared_ptr<T> entity, GameSection section = None) {
		// if the drawable with given id already exists, don't add anything
		if (entity_map.find(id) != entity_map.end()) return nullptr;
		entity_map.insert({ id, entity });

		add_entity_raw(entity, section);

		return entity;
	}

	void remove_entity(string id) {
		if (entity_map.find(id) == entity_map.end()) return;
		entity_map.erase(id);
	}

	// same as above "add", except the entity isn't added to the table
	template <typename T>
	shared_ptr<T> add_entity_raw(shared_ptr<T> entity, GameSection section = None) {
		entities.push_back(entity);
		associate_with_section(section, entity);
		return entity;
	}

	// schedules an entity pointer to delete in the next frame
	void schedule_to_delete(Drawable* entity_ptr) {
		entities_to_delete.push_back(entity_ptr);
	}

	// deletes all scheduled for remove entities
	void delete_scheduled() {
		for (Drawable* ent_to_remove : entities_to_delete) {
			// erases all "deleted" entities past the new returned end() iterator
			entities.erase(
				// remove_if moves all "deleted" entities to the end and shifts the 
				// end() iterator back after the "non-deleted" entities.
				// it returns the new end() iterator
				remove_if(
					entities.begin(), 
					entities.end(), 
					[&](const shared_ptr<Drawable>& ent_ptr) {
						return ent_ptr.get() == ent_to_remove;
					}
				),
				entities.end()
			);

			// erase the entity from all associated sections to remove it completely
			for (auto& section_pair : section_map) {
				auto& section_vec = section_pair.second;

				section_vec.erase(
					remove_if(
						section_vec.begin(),
						section_vec.end(),
						[&](const shared_ptr<Drawable>& ent_ptr) {
							return ent_ptr.get() == ent_to_remove;
						}
					),
					section_vec.end()
				);
			}
		}

		entities_to_delete.clear();
	}

	// associates a given entity with a GameSection, allowing filtering entites by GameSection
	void associate_with_section(GameSection section, shared_ptr<Drawable> entity) {
		auto& section_vec = section_map.at(section);

		// if the entity is already associated with a section, don't add it again
		if (find(section_vec.begin(), section_vec.end(), entity) != section_vec.end())
			return;

		section_vec.push_back(entity);
	}

	// disassociate a given entity from all GameSections
	void disassociate_from_sections(shared_ptr<Drawable> entity) {
		for (auto& section_pair : section_map) {
			auto& section_vec = section_pair.second;
			auto found_it = find(section_vec.begin(), section_vec.end(), entity);
			if (found_it != section_vec.end())
				section_vec.erase(found_it);
		}
	}

	// disassociate a given entity from a given GameSection
	void disassociate_from_section(GameSection section, shared_ptr<Drawable> entity) {
		auto& section_vec = section_map.at(section);
		auto found_it = find(section_vec.begin(), section_vec.end(), entity);
		if (found_it != section_vec.end())
			section_vec.erase(found_it);
	}
};

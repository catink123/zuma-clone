#pragma once
#include "../engine/Texture.h"
#include <vector>
#include "Player.h"
#include "Balls.h"
#include "LevelData.h"

using namespace std;

class Level : public Drawable, public Updatable {
	shared_ptr<AssetManager> asset_manager;
	shared_ptr<EntityManager> entity_manager;

	shared_ptr<Player> player = nullptr;
	shared_ptr<BallTrack> ball_track = nullptr;
	shared_ptr<Sprite> background_sprite = nullptr;
	SDL_Renderer* renderer;

public:
	const LevelData* data;

	Level(
		LevelData* level_data,
		shared_ptr<AssetManager> asset_manager,
		shared_ptr<EntityManager> entity_manager,
		SDL_Renderer* renderer,
		function<void(shared_ptr<EntityManager>, shared_ptr<AssetManager>, SDL_Renderer*)> create_ui
	) : data(level_data), asset_manager(asset_manager), entity_manager(entity_manager), renderer(renderer) {

		background_sprite = entity_manager->add_entity(
			"level_bg",
			make_shared<Sprite>(
				data->background,
				vec2(), vec2(1), 0.0F,
				nullopt, vec2(WINDOW_WIDTH, WINDOW_HEIGHT)
			),
			InLevel
		);

		ball_track = entity_manager->add_entity(
			"ball_track",
			make_shared<BallTrack>(
				data->track_points,
				data->track_ball_count,
				asset_manager,
				entity_manager
			),
			InLevel
		);

		ball_track->speed_multiplier = data->track_speed_multiplier;

		create_ui(entity_manager, asset_manager, renderer);

		player = entity_manager->add_entity(
			"player", 
			make_shared<Player>(
				asset_manager->get_texture("player_normal"), 
				asset_manager->get_texture("player_action"),
				asset_manager,
				entity_manager,
				ball_track
			),
			InLevel
		);

		if (player != nullptr) {
			player->local_transform.scale = 0.75;
			player->global_transform.position = data->player_position;
		}

		SoundManager::set_music(asset_manager->get_audio("level_song"));
	}

	~Level() {
		entity_manager->remove_entity("level_bg");
		entity_manager->remove_entity("player");
		entity_manager->remove_entity("ball_track");
		entity_manager->remove_entity("game_ui");
	}

	void draw(SDL_Renderer*, const RendererState&) const override {}

	void update(const float& delta, GameState& game_state) override {
		auto ui = entity_manager->get_entity_by_name<UI>("game_ui");
		shared_ptr<Text> score_text = dynamic_pointer_cast<Text>(ui->root_element->children[1]);
		score_text->set_content(string("Score: ") + to_string(game_state.game_score));
	}
};

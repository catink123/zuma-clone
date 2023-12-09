#pragma once
#include <random>
#include <cmath>
#include "../engine/Sprite.h"
#include "../engine/AssetManager.h"
#include "../engine/EntityManager.h"
#include "GameState.h"
#include "Balls.h"

// PlayerBall is used with a Player
// it has a velocity parameter for the PlayerBall to be able to shoot
class PlayerBall : public Ball {
	static constexpr float BALL_SPEED = 400.0F;
	vec2 velocity;
	// needed to check for a collision between the BallTrack and the PlayerBall
	shared_ptr<BallTrack> ball_track = nullptr;
	// needed for PlayerBall to delete itself upon collision
	shared_ptr<EntityManager> entity_manager = nullptr;

	// animation for when this ball collides with the BallTrack and is inserted into it
	Animation* insertion_animation = nullptr;
	float saved_angle = 0;
	float target_angle = 0;
	uint target_ball_segment_index = 0;
	bool is_inserting_at_end = false;

	void set_insertion_animation(const uint& ball_segment_index, bool inserting_at_end);

public:

	bool collision_enabled = true;

	PlayerBall(
		shared_ptr<AssetManager> asset_manager,
		shared_ptr<EntityManager> entity_manager,
		shared_ptr<BallTrack> ball_track,
		BallColor color,
		const vec2& position = vec2(),
		const float& rotation = 0
	)
		: Ball(asset_manager, color, position), entity_manager(entity_manager), ball_track(ball_track)
	{ this->global_transform.rotation = rotation; }

	void update(const float& delta, GameState& game_state) override;

	void shoot(const float& angle);
};

class Player : public Sprite, public Updatable {
	static const int PLAYER_ANIM_SHIFT = 30;
	static constexpr float MOUSE_BUTTON_DELAY = 1.0F;

	// the default texture used
	Texture& normal_texture;
	// the texture used when LMB is pressed
	Texture& action_texture;

	// animation when LMB is pressed
	Animation* current_animation = nullptr;

	// animation when a new "holding" ball appears
	Animation* drawing_ball_animation = nullptr;

	// timers for delaying the mouse button presses
	Timer* lmb_timer = nullptr;
	Timer* rmb_timer = nullptr;

	void shoot_ball();

	void swap_balls();

public:
	shared_ptr<Ball> drawing_ball = nullptr;
	BallColor primary_color;
	optional<BallColor> secondary_color;
	// needed to load textures in balls
	shared_ptr<AssetManager> asset_manager = nullptr;
	// needed to add PlayerBall entities and control them
	shared_ptr<EntityManager> entity_manager = nullptr;
	shared_ptr<BallTrack> ball_track = nullptr;

	Player(
		Texture& normal_texture, 
		Texture& action_texture,
		shared_ptr<AssetManager> asset_manager,
		shared_ptr<EntityManager> entity_manager,
		shared_ptr<BallTrack> ball_track
	);

	void draw(SDL_Renderer* renderer, const RendererState& renderer_state) const override;

	void update(const float& delta, GameState& game_state) override;
};

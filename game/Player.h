#pragma once
#include "../engine/Sprite.h"
#include "../engine/AssetManager.h"
#include "../engine/EntityManager.h"
#include "GameState.h"
#include "Balls.h"
#include <random>
#include <cmath>

// PlayerBall is used with a Player
// it has a velocity parameter for the PlayerBall to be able to shoot
class PlayerBall : public Ball {
	static const int BALL_SPEED = 60;
	vec2 velocity;
	// needed to check for a collision between the BallTrack and the PlayerBall
	shared_ptr<BallTrack> ball_track;

public:

	PlayerBall(
		shared_ptr<AssetManager> asset_manager,
		shared_ptr<BallTrack> ball_track,
		BallColor color,
		const vec2& position = vec2(),
		const float& rotation = 0
	)
		: Ball(asset_manager, color, position), ball_track(ball_track)
	{ this->global_transform.rotation = rotation; }

	void update(const float& delta, GameState& game_state) override;

	void shoot(const float& angle);
};

class Player : public Sprite, public Updatable {
	static const int PLAYER_ANIM_SHIFT = 30;
	static const int MOUSE_BUTTON_DELAY = 1;

	// the default texture used
	const Texture& normal_texture;
	// the texture used when LMB is pressed
	const Texture& action_texture;

	// animation when LMB is pressed
	Animation* current_animation = nullptr;

	// timers for delaying the mouse button presses
	Timer* lmb_timer = nullptr;
	Timer* rmb_timer = nullptr;

	void shoot_ball();

public:
	shared_ptr<Ball> primary_ball = nullptr;
	shared_ptr<Ball> secondary_ball = nullptr;
	// needed to load textures in balls
	shared_ptr<AssetManager> asset_manager = nullptr;
	// needed to add PlayerBall entities and control them
	shared_ptr<EntityManager> entity_manager = nullptr;
	shared_ptr<BallTrack> ball_track = nullptr;

	Player(
		const Texture& normal_texture, 
		const Texture& action_texture,
		shared_ptr<AssetManager> asset_manager,
		shared_ptr<EntityManager> entity_manager,
		shared_ptr<BallTrack> ball_track
	);

	void draw(SDL_Renderer* renderer, const RendererState& renderer_state) const override;

	void update(const float& delta, GameState& game_state) override;
};

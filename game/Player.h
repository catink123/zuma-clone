#pragma once
#include "../engine/Sprite.h"
#include "Balls.h"
#include "../engine/AssetManager.h"
#include <random>
#include "../game/GameState.h"
#include <cmath>

// PlayerBall is used with a Player
// it has a velocity parameter for the PlayerBall to be able to shoot
class PlayerBall : public Ball {
	static const int BALL_SPEED = 10;
	vec2 velocity;
	// needed to check for a collision between the BallTrack and the PlayerBall
	shared_ptr<BallTrack> ball_track;

public:
	PlayerBall(
		shared_ptr<AssetManager> asset_manager,
		shared_ptr<BallTrack> ball_track,
		BallColor color,
		const vec2& position,
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

	// timer for delaying the mouse button press
	Timer* mb_timer = nullptr;

	void shoot_ball();

public:
	unique_ptr<PlayerBall> primary_ball;
	unique_ptr<PlayerBall> secondary_ball;
	shared_ptr<AssetManager> asset_manager;
	shared_ptr<BallTrack> ball_track;

	Player(
		const Texture& normal_texture, 
		const Texture& action_texture,
		shared_ptr<AssetManager> asset_manager,
		shared_ptr<BallTrack> ball_track
	);

	void draw(SDL_Renderer* renderer, const RendererState& renderer_state) const override;

	void update(const float& delta, GameState& game_state) override;
};

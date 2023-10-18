#pragma once
#include "../engine/Sprite.h"
#include "Balls.h"
#include "../engine/AssetManager.h"
#include <random>
#include "../game/GameState.h"
#include <cmath>

class Player : public Sprite, public Animatable {
	static const int PLAYER_ANIM_SHIFT = 30;

	pair<Texture*, Texture*> player_textures;
	Texture* normal_texture;
	Texture* action_texture;
	GameState* game_state;

	Animation* current_animation = nullptr;
	vec2 saved_position;

public:
	BallColor primary_ball;
	BallColor secondary_ball;

	Player(pair<Texture*, Texture*> player_textures, GameState* game_state);

	void draw(SDL_Renderer* renderer, const RendererState& renderer_state);
	void update(const float& delta);
};

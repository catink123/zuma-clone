#pragma once
#include "Sprite.h"
#include "Balls.h"
#include "AssetManager.h"
#include <random>
#include "GameState.h"
#include <cmath>

class Player : public Sprite {
	pair<Texture*, Texture*> player_textures;
	Texture* normal_texture;
	Texture* action_texture;
	GameState* game_state;
public:
	BallColor primary_ball;
	BallColor secondary_ball;

	Player(pair<Texture*, Texture*> player_textures, GameState* game_state);

	void draw(SDL_Renderer* renderer, const RendererState& renderer_state);
};

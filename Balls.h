#pragma once
#include <unordered_map>
#include <string>
#include <cmath>
#include "Sprite.h"
#include "AssetManager.h"

enum BallColor {
	Red,
	Green,
	Blue,
	Yellow,
	Gray,
	Purple
};

static const int BALL_COLOR_COUNT = 6;

static const std::unordered_map<BallColor, std::string> BALL_COLOR_TEXTURE_MAP = {
	{ BallColor::Red,		"ball_red" },
	{ BallColor::Green,		"ball_green" },
	{ BallColor::Blue,		"ball_blue" },
	{ BallColor::Yellow,	"ball_yellow" },
	{ BallColor::Gray,		"ball_gray" },
	{ BallColor::Purple,	"ball_purple" }
};

class Ball : public Sprite {
	static const uint BALL_SIZE = 25;
	static const uint BALL_ROTATION_FRAME_COUNT = 30;
	BallColor color;
	float ball_angle;

public:
	Ball(
		AssetManager& asset_manager, 
		BallColor color,
		vec2 position
	)
		: 
		Sprite(
			asset_manager.get_texture(
				BALL_COLOR_TEXTURE_MAP.at(color)
			),
			position
		),
		color(color), ball_angle(0) {}

	void draw(SDL_Renderer* renderer);

	float get_ball_angle() const;
	void set_ball_angle(float angle);

	// static SDL_Rect get_
};

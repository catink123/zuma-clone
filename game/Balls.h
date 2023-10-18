#pragma once
#include <unordered_map>
#include <string>
#include <cmath>
#include <memory>
#include "../engine/Sprite.h"
#include "../engine/AssetManager.h"

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
	static const uint BALL_SIZE = 50;
	static const uint BALL_ROTATION_FRAME_COUNT = 100;
	static const uint BALL_SPRITESHEET_H = 10;
	BallColor color;
	float ball_angle;

public:
	Ball(
		shared_ptr<AssetManager> asset_manager,
		BallColor color,
		vec2 position
	);

	void draw(SDL_Renderer* renderer, const RendererState& renderer_state);

	float get_ball_angle() const;
	void set_ball_angle(float angle);

	// static SDL_Rect get_
};

struct TrackSegment {
	float angle = 0;
	float angle_cos = 0;
	float angle_sin = 0;
	float length = 0;
};

struct BallTrackCache {
	vector<vec2> points;
	vector<TrackSegment> segments;
	float total_length = 0;
};

struct BallSegment {
	vector<Ball> balls;
	float position = 0;
};

class BTCreationException : public exception {
	const char* msg;
public:
	BTCreationException(const char* message);
	const char* what();
};

class BallTrack {
	BallTrackCache cache;
	vector<BallSegment> ball_segments;
	BallTrack(const vector<vec2>& points);
};

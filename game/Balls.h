#pragma once
#include <unordered_map>
#include <string>
#include <cmath>
#include <memory>
#include "../engine/Sprite.h"
#include "../engine/AssetManager.h"
#include <random>

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
	static const uint BALL_ROTATION_FRAME_COUNT = 100;
	static const uint BALL_SPRITESHEET_H = 10;
	BallColor color;
	float ball_angle;

public:
	static const uint BALL_SIZE = 50;

	Ball(
		shared_ptr<AssetManager> asset_manager,
		BallColor color,
		vec2 position = vec2(0, 0)
	);

	void draw(SDL_Renderer* renderer, const RendererState& renderer_state);

	float get_ball_angle() const;
	void set_ball_angle(float angle);
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

	float get_total_length() const;
};

class BTCreationException : public exception {
	const char* msg;
public:
	BTCreationException(const char* message);
	const char* what();
};

class BallTrack : public Drawable, public Animatable {
	BallTrackCache cache;
	vector<BallSegment> ball_segments;
	shared_ptr<AssetManager> asset_manager;

	int get_track_segment_by_position(const float& position) const;
	float get_track_segment_length_sum(const uint& last_segment) const;

public:
	BallTrack(const vector<vec2>& points, shared_ptr<AssetManager> asset_manager);
	void draw(SDL_Renderer* renderer, const RendererState& renderer_state);
	void update(const float& delta);

	bool is_point_on_balltrack(const vec2& point, const float& radius) const;
};

#pragma once
#include <unordered_map>
#include <set>
#include <string>
#include <cmath>
#include <memory>
#include <optional>
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

// ball color to texture name correspondence
static const std::unordered_map<BallColor, std::string> BALL_COLOR_TEXTURE_MAP = {
	{ BallColor::Red,		"ball_red" },
	{ BallColor::Green,		"ball_green" },
	{ BallColor::Blue,		"ball_blue" },
	{ BallColor::Yellow,	"ball_yellow" },
	{ BallColor::Gray,		"ball_gray" },
	{ BallColor::Purple,	"ball_purple" }
};

inline BallColor get_random_ball_color() {
	return (BallColor)(rand() % BALL_COLOR_COUNT);
}

// the logic behind the ball that can rotate and spin with animation
class Ball : public Sprite, public Updatable {
	static const uint BALL_ROTATION_FRAME_COUNT = 100;	// frame count of the ball's spritesheet
	static const uint BALL_SPRITESHEET_H = 10;			// ball count of the spritesheet's column
	float ball_angle;	// ball rotation around it's X axis
	shared_ptr<Sprite> sheen_sprite = nullptr;

public:
	shared_ptr<AssetManager> asset_manager = nullptr;
	static const uint BALL_SIZE = 50; // physical ball size
	BallColor color;

	Ball(
		shared_ptr<AssetManager> asset_manager, // used for importing the specific ball textures
		BallColor color,
		vec2 position = vec2(0, 0)
	);

	void draw(SDL_Renderer* renderer, const RendererState& renderer_state) const override;

	void update(const float& delta, GameState& game_state) override;

	// public getter for the private ball_angle
	float get_ball_angle() const;
	// public setter for the private ball_angle with conversion to 0-360deg format
	void set_ball_angle(const float& angle);

	// change color to given one and replace the used texture
	void change_color(BallColor new_color);
};

struct TrackSegment {
	float angle = 0;
	float angle_cos = 0;
	float angle_sin = 0;
	float length = 0;
};

// cache for precomputed values to quickly draw the ball track
struct BallTrackCache {
	vector<vec2> points;
	vector<TrackSegment> segments;
	float total_length = 0;
};

struct BallSegment {
	vector<Ball> balls;
	float position = 0;

	uint get_total_length() const;
};

// collision data for use with the BallTrack::get_collision_data function
struct BallTrackCollisionData {
	uint track_segment_index;
	uint ball_segment_index;
	float track_segment_position;
	float ball_segment_position;
};

// exception thrown upon construction of BallTrack (i.e. there are less than 2 points for the track)
class BTCreationException : public exception {
	const char* msg;
public:
	BTCreationException(const char* message);
	const char* what();
};

// logic behind drawing the balls on a track and checking collision with segments of balls
class BallTrack : public Drawable, public Updatable {
	static constexpr float BASE_SPEED = 40.0F;
	// pointer to asset_manager for Ball's constructor
	shared_ptr<AssetManager> asset_manager;

	// find track segment's index by a given ball segment's position
	uint get_track_segment_by_position(const float& position) const;
	// get all track segment indecies that a given BallSegment goes through
	vector<uint> get_track_segments_from_ball_segment(const BallSegment& ball_segment) const;
	// calculate total track length up to (and including) the segment, 
	// index of which is given as an argument
	float get_track_segment_length_sum(const uint& last_segment) const;

public:
	// cache for precomputing track segment values (like angle, cosine, sine and length)
	BallTrackCache cache;
	// segments with filled balls that move over time
	vector<BallSegment> ball_segments;

	float speed_multiplier = 1;

	BallTrack(const vector<vec2>& points, shared_ptr<AssetManager> asset_manager);
	void draw(SDL_Renderer* renderer, const RendererState& renderer_state) const override;
	virtual void update(const float& delta, GameState& game_state) override;

	// check for collision with the ball track at a given point 
	// and radius of collision (ball's and/or track's radius)
	// returns nullopt if there was no collision
	optional<BallTrackCollisionData> get_collision_data(const vec2& point, const float& point_radius) const;

	// splits a ball segment into two parts by it's index and position
	void cut_ball_segment(const uint& ball_segment_index, const float& position);
};

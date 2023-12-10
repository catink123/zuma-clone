#pragma once
#include <unordered_map>
#include <set>
#include <string>
#include <cmath>
#include <memory>
#include <optional>
#include "../engine/Sprite.h"
#include "../engine/AssetManager.h"
#include "../engine/EntityManager.h"
#include "../engine/SoundManager.h"
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
	bool show = true;

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
	Timer* shift_timer = nullptr;

	vector<Ball> balls;
	float position = 0;
	float speed = 0;
	bool is_shifting = false;

	void shift();

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

static const std::unordered_map<BallColor, SDL_Color> BALLCOLOR_TO_RGB_MAP = {
	{ BallColor::Red,		SDL_Color({ 255, 0, 0 }) },
	{ BallColor::Green,	SDL_Color({ 0, 255, 0 }) },
	{ BallColor::Blue,		SDL_Color({ 0, 0, 255 }) },
	{ BallColor::Yellow,	SDL_Color({ 255, 255, 0 }) },
	{ BallColor::Gray,		SDL_Color({ 255, 255, 255 }) },
	{ BallColor::Purple,	SDL_Color({ 255, 0, 255 }) }
};

class BallParticles : public Drawable, public Updatable {
	static const uint MIN_COUNT = 10;
	static const uint MAX_COUNT = 20;
	static constexpr float MIN_VELOCITY = 10.0F;
	static constexpr float MAX_VELOCITY = 200.0F;
	static constexpr float SIZE = 10.0F;
	static constexpr float GRAVITY = 100.0F;

	struct Particle {
		vec2 position;
		vec2 velocity;
	};
	vector<Particle> particles;
	SDL_Color color;
	Texture* texture;

	void create_particle(vec2 origin) {
		Particle p;

		p.position = origin;
		
		float random_pos_angle = rand_float() * M_PI * 2;
		float random_pos_len = rand_float() * static_cast<float>(Ball::BALL_SIZE) / 2;
		p.position.x += cosf(random_pos_angle) * random_pos_len;
		p.position.y += sinf(random_pos_angle) * random_pos_len;

		p.velocity.x = (rand_float() * (MAX_VELOCITY - MIN_VELOCITY) + MIN_VELOCITY) * (rand() % 2 == 1 ? -1 : 1);
		p.velocity.y = (rand_float() * (MAX_VELOCITY - MIN_VELOCITY) + MIN_VELOCITY) * (rand() % 2 == 1 ? -1 : 1);

		particles.push_back(p);
	}

public:
	BallParticles(
		vec2 origin,
		BallColor color,
		Texture* particle_texture
	) :
		color(BALLCOLOR_TO_RGB_MAP.at(color)),
		texture(particle_texture) 
	{
		for (int i = 0; i < (rand() % (MAX_COUNT - MIN_COUNT) + MIN_COUNT); i++)
			create_particle(origin);
	}

	void draw(SDL_Renderer* renderer, const RendererState& renderer_state) const override {
		for (const auto& p : particles) {
			auto rect = texture->get_rect(p.position.x - SIZE / 2, p.position.y - SIZE / 2);
			rect.w = SIZE;
			rect.h = SIZE;

			rect.x *= renderer_state.scaling;
			rect.y *= renderer_state.scaling;
			rect.w *= renderer_state.scaling;
			rect.h *= renderer_state.scaling;
			SDL_SetTextureColorMod(texture->get_raw(), color.r, color.g, color.b);

			SDL_RenderCopyExF(renderer, texture->get_raw(), nullptr, &rect, 0, nullptr, SDL_FLIP_NONE);

			SDL_SetTextureColorMod(texture->get_raw(), 255, 255, 255);
		}
	}

	void update(const float& delta, GameState& game_state) {
		for (int i = 0; i < particles.size(); i++) {
			Particle& p = particles[i];

			p.position.x += p.velocity.x * delta;
			p.position.y += p.velocity.y * delta;

			if (p.position.x > WINDOW_WIDTH + SIZE || p.position.x < -SIZE || p.position.y > WINDOW_HEIGHT + SIZE) {
				particles.erase(particles.begin() + i);
			}
			else {
				p.velocity.y += GRAVITY * delta;
			}
		}
	}

	bool is_done() const { return particles.size() == 0; }
};

// logic behind drawing the balls on a track and checking collision with segments of balls
class BallTrack : public Drawable, public Updatable {
	static constexpr float BASE_SPEED = 40.0F;
	static constexpr float SEGMENT_COLLISION_ERROR = 1.0F;
	static constexpr float SEGMENT_FOLLOW_ACCELERATION = 400.0F;
	static constexpr float FAIL_SEGMENT_ACCELERATION = 50.0F;
	static const uint SCORE_PER_BALL = 50;

	// if some ball hits the death window, this is set to true...
	bool is_failing = false;
	// ... and this is set to true when all the balls are gone
	bool is_fading_out_to_screen = false;

	// pointer to asset_manager for Ball's constructor
	shared_ptr<AssetManager> asset_manager;

	// ball breaking particles
	vector<BallParticles> ball_particles;

	unique_ptr<Sprite> death_window = nullptr;

	// find track segment's index by a given ball segment's position
	optional<uint> get_track_segment_by_position(const float& position) const;
	// get all track segment indecies that a given BallSegment goes through
	vector<uint> get_track_segments_from_ball_segment(const BallSegment& ball_segment) const;
	// calculate total track length up to (and including) the segment, 
	// index of which is given as an argument
	float get_track_segment_length_sum(const uint& last_segment) const;

public:
	static constexpr float BALL_INSERTION_TIME = 0.25F;

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
	// and returns true if the segment was cut and false otherwise
	bool cut_ball_segment(const uint& ball_segment_index, const float& position, const float& spacing = 0);

	// connects a ball segment by it's index with the next ball segment
	void connect_ball_segments(const uint& ball_segment_index, bool inherit_seconds_speed = false);

	// adds a new ball to the specified ball segment index and position
	void add_insertion_space(const uint& ball_segment_index, const float& position);

	void insert_new_ball(const uint& ball_segment_index, BallColor color, bool inserting_at_end);

	// finds a TrackSegment by the end of the BallSegment of given index
	//
	// needed for ball insertion animation
	const TrackSegment& get_track_segment_by_bs_index(const float& ball_segment_index);

	// calculates a vec2 of the destination insertion point,
	// positioned globally (relative to window)
	//
	// needed for ball insertion animation
	vec2 get_insertion_pos_by_bs_index(const float& ball_segment_index, bool inserting_at_end);
};

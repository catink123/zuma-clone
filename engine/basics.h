#pragma once
#include <SDL.h>
#include "common.h"
#include "../game/GameState.h"
#include "Animation.h"

class Transform {
public:
	vec2 position;
	vec2 scale;
	float rotation = 0;

	Transform(
		vec2 position = vec2(0, 0), 
		vec2 scale = vec2(1, 1), 
		float rotation = 0
	) : position(position), scale(scale), rotation(rotation) {}

	Transform(
		float x, 
		float y, 
		float x_scale = 1, 
		float y_scale = 1, 
		float rotation = 0
	) : position(vec2(x, y)), scale(vec2(x_scale, y_scale)), rotation(rotation) {}

	Transform operator+(const Transform& other) const;
};

class Drawable {
public: 
	virtual void draw(SDL_Renderer* renderer, const RendererState& renderer_state) const = 0;
};

class Updatable {
public:
	virtual void update(const float& delta, GameState& game_state) = 0;
};

// Timer represents a basic timer
class Timer : public Updatable {
	float duration;
	float current_time = 0;

public:
	Timer(const float& duration) : duration(duration) {}

	// returns if the current_time is over the duration of the timer
	bool is_done() const;
	// resets the time to zero and optionally adds 
	// the overrun time (difference between duration and current_time)
	void reset(bool save_overrun);

	// sets the current progress with a percentage
	void set_progress(const float& progress);

	const float& get_current_time() const;

	void update(const float& delta, GameState& game_state) override;
};

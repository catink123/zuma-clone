#pragma once
#include <functional>
#include <map>
#include <cmath>

typedef std::function<float(const float&)> TimingFunction;

enum TimingFunctionType {
	Linear, EaseIn, EaseOut, EaseInOut
};

// timing functions by their type
const std::map<TimingFunctionType, TimingFunction> TIMING_FUNCTIONS = {
	{ Linear, [](const float& t) { return t; } },
	{ EaseIn, [](const float& t) { return powf(t, 2); } },
	{ EaseOut, [](const float& t) { return 1 - powf(1 - t, 2); } },
	{ EaseInOut, [](const float& t) { return t < 0.5F ? (powf(t * 2.0F, 2.0F) / 2.0F) : (0.5F + (1 - powf((1 - t) * 2, 2)) / 2); } }
};

// how the animation is repeated
enum AnimationLoopFill {
	Repeat,
	Reverse
};

class Animation {
	float current_time = 0;
	int current_iteration = 0;
	bool is_anim_finished = false;

	// update the loop iteration
	void loop_update();
public:
	int loop_count = -1;
	float duration = 0;
	TimingFunction timing_function = nullptr;
	AnimationLoopFill loop_fill = Repeat;

	Animation(
		float duration,
		TimingFunction timing_function = TIMING_FUNCTIONS.at(Linear),
		int loop_count = 0,
		AnimationLoopFill loop_fill = Repeat
	) : duration(duration), timing_function(timing_function), loop_count(loop_count), loop_fill(loop_fill) {}

	void update(const float& delta);
	void set_progess(const float& progress);
	float get_progress() const;
	const bool& is_finished() const;
};

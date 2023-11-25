#include "../engine/Animation.h"

void Animation::loop_update() {
	// if the animation time exceeded it's duration...
	if (current_time > duration) {
		// ... and loop count is infinite (has a negative value), repeat the animation
		if (loop_count < 0) {
			current_time -= duration;
		}
		// otherwise, bump up iteration count and check if the animation should be finished
		else {
			current_time -= duration;
			current_iteration++;
			
			is_anim_finished = current_iteration > loop_count;
		}
	}
}

void Animation::update(const float& delta) {
	// we shouldn't update the time if the animation is finished
	if (is_anim_finished) return;
	current_time += delta;
	loop_update();
}

void Animation::set_progess(const float& progress) {
	// seek the animation time relative to the duration of the animation
	current_time = progress * duration;
	loop_update();
}

float Animation::get_progress() const {
	// progress is bounded to 0-1
	float progress = current_time / duration;
	// if the current loop fill is Reverse, invert the progress to reverse the animation
	if (current_iteration % 2 == 1 && loop_fill == AnimationLoopFill::Reverse)
		progress = 1 - progress;
	// if no timing function is specified, return the linear representation of the progress
	if (timing_function == nullptr)
		return progress;
	// otherwise, pass the progress through the specified timing function
	return timing_function(progress);
}

const bool& Animation::is_finished() const {
	return is_anim_finished;
}
#include "../engine/Animation.h"

void Animation::loop_update() {
	if (current_time > duration) {
		if (loop_count < 0) {
			current_time -= duration;
		}
		else {
			current_time -= duration;
			current_iteration++;
			
			is_anim_finished = current_iteration > loop_count;
		}
	}
}

void Animation::update(const float& delta) {
	if (is_anim_finished) return;
	current_time += delta;
	loop_update();
}

void Animation::set_progess(const float& progress) {
	current_time = progress * duration;
	loop_update();
}

float Animation::get_progress() const {
	float progress = current_time / duration;
	if (timing_function == nullptr)
		return progress;
	return timing_function(progress);
}

const bool& Animation::is_finished() const {
	return is_anim_finished;
}
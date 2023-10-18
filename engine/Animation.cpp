#include "../engine/Animation.h"

void Animation::update(const float& delta) {
	if (is_finished) return;
	current_time += delta;
	if (current_time > duration) {
		if (loop_count < 0) {
			current_time -= duration;
		}
		else {
			current_time -= duration;
			current_iteration++;
			
			is_finished = current_iteration > loop_count;
		}
	}
}

float Animation::get_progress() const {
	float progress = current_time / duration;
	if (timing_function == nullptr)
		return progress;
	return timing_function(progress);
}
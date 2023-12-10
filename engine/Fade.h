#pragma once
#include "Sprite.h"

class Fade : public Sprite, public Updatable {
	Animation* fade_animation = nullptr;
	function<void(void)> callback_func;

	bool is_fading_in = false;

	void setup_animation(const float& duration) {
		if (fade_animation == nullptr)
			delete fade_animation;

		fade_animation = new Animation(duration);
	}
public:
	static constexpr float DURATION = 1.0F;

	Fade(Texture* texture) : Sprite(texture, vec2(), vec2(1), 0, nullopt, vec2(WINDOW_WIDTH, WINDOW_HEIGHT)) {
		opacity = 0;
	}

	void fade_in(function<void(void)> callback, float duration = DURATION) {
		callback_func = callback;
		is_fading_in = true;
		setup_animation(duration);
	}

	void fade_out(function<void(void)> callback, float duration = DURATION) {
		callback_func = callback;
		is_fading_in = false;
		setup_animation(duration);
	}

	bool is_fade_done() {
		return fade_animation == nullptr || fade_animation->is_finished();
	}

	void update(const float& delta, GameState&) override {
		if (fade_animation) {
			if (fade_animation->is_finished()) {
				delete fade_animation;
				fade_animation = nullptr;

				if (is_fading_in)
					opacity = 1;
				else
					opacity = 0;

				callback_func();
			}
			else {
				const float& progress = fade_animation->get_progress();
				if (is_fading_in)
					opacity = progress;
				else
					opacity = 1 - progress;

				fade_animation->update(delta);
			}
		}
	}
};
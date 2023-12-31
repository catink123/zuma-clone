#include "Player.h"

Player::Player(
	Texture& normal_texture,
	Texture& action_texture,
	shared_ptr<AssetManager> asset_manager,
	shared_ptr<EntityManager> entity_manager,
	shared_ptr<BallTrack> ball_track
) :
	Sprite(&normal_texture, vec2(10, 10)),
	normal_texture(normal_texture),
	action_texture(action_texture),
	asset_manager(asset_manager),
	entity_manager(entity_manager),
	ball_track(ball_track)
{
	primary_color = get_random_ball_color();
	secondary_color = get_random_ball_color();

	drawing_ball = 
		entity_manager->add_entity_raw(
			make_shared<Ball>(asset_manager, primary_color, vec2(0, 100)), InLevel
		);
	drawing_ball->origin_transform = &global_transform;

	secondary_drawing_ball =
		entity_manager->add_entity_raw(
			make_shared<Ball>(asset_manager, secondary_color.value(), vec2(0, -50)), InLevel
		);
	secondary_drawing_ball->origin_transform = &global_transform;
	secondary_drawing_ball->global_transform.scale = 0.5;

	vertical_alignment = VerticalAlignment::Middle;
	horizontal_alignment = HorizontalAlignment::Center;
}

void Player::shoot_ball() {
	// create the shooting ball
	auto shooting_ball =
		entity_manager->add_entity_raw(
			make_shared<PlayerBall>(asset_manager, entity_manager, ball_track, primary_color), InLevel
		);

	// copy all transforms from primary_ball to shooting_ball
	shooting_ball->global_transform = drawing_ball->global_transform;
	shooting_ball->local_transform = drawing_ball->local_transform;
	shooting_ball->origin_transform = drawing_ball->origin_transform;

	// "flattens" origin transform to global transform
	shooting_ball->apply_origin_transform();
	// shoot the newly created PlayerBall
	shooting_ball->shoot(global_transform.rotation + 90);

	// "shift" balls forward and assign a new color to the secondary ball

	auto available_colors = ball_track->get_current_colors();

	// swap balls places if there is a secondary color
	if (secondary_color)
		swap_balls();
	else {
		primary_color = pick_random(available_colors);
		drawing_ball->change_color(primary_color);
	}

	// create a new color and assign it to secondary_ball
	secondary_color = pick_random(available_colors);
	secondary_drawing_ball->change_color(secondary_color.value());

	if (drawing_ball_animation == nullptr)
		drawing_ball_animation = new Animation(0.5, TIMING_FUNCTIONS.at(EaseOut));

	drawing_ball_animation->set_progess(0);
}

void Player::swap_balls() {
	if (secondary_color)
		swap(primary_color, secondary_color.value());

	drawing_ball->change_color(primary_color);
	secondary_drawing_ball->change_color(secondary_color.value());
}

void Player::draw(SDL_Renderer* renderer, const RendererState& renderer_state) const {
	drawing_ball->draw(renderer, renderer_state);

	Sprite::draw(renderer, renderer_state);

	if (secondary_color)
		secondary_drawing_ball->draw(renderer, renderer_state);
}


void Player::update(const float& delta, GameState& game_state) {
	// if there's an animation in progress, update it
	if (current_animation)
		current_animation->update(delta);

	if (drawing_ball_animation)
		drawing_ball_animation->update(delta);

	// calculate player's rotation using trigonometry
	vec2 size = get_size();
	float w = (global_transform.position.x) - game_state.mouse_state.mouse_pos.x;
	float h = (global_transform.position.y) - game_state.mouse_state.mouse_pos.y;
	global_transform.rotation = rad_to_deg(atan(h / w)) + 90;
	// flip the angle if W was negative, as cmath's atan doesn't return negative values
	if (w < 0) global_transform.rotation += 180;

	// if the LMB is pressed down, sping back the player and shoot the ball
	if (game_state.mouse_state.is_lmb_down && !game_state.mouse_state.mouse_on_ui) {
		// if the texture isn't already the action one, change it
		if (*get_texture() != action_texture) change_texture(&action_texture);

		// if there was no timer, then LMB was just pressed, create a new timer
		// and allow the swap action right away by setting progress to 100%
		if (lmb_timer == nullptr) {
			lmb_timer = new Timer(MOUSE_BUTTON_DELAY);
			lmb_timer->set_progress(1);
		}

		// if the timer is done counting, allow the LMB action and reset the timer
		if (lmb_timer->is_done()) {
			// if there's no current animation, create it
			if (current_animation == nullptr) {
				current_animation =
					new Animation(
						0.25, // duration
						TIMING_FUNCTIONS.at(TimingFunctionType::EaseOut) // EaseOut timing function
					);
			}
			
			// reset the animation progress in any case
			current_animation->set_progess(0);

			// shoot the ball
			shoot_ball();

			lmb_timer->reset(true);
		}
	}
	// otherwise LMB is not pressed down, change the texture back to normal
	else {
		// if the LMB is not pressed down, remove the delay timer to set up for the next LMB press
		if (lmb_timer && lmb_timer->is_done()) {
			delete lmb_timer;
			lmb_timer = nullptr;
		}

		if (*get_texture() != normal_texture) change_texture(&normal_texture);
	}

	// if the RMB is pressed down, swap primary and secondary bots
	// and delay the next swap if it's still pressed down
	if (game_state.mouse_state.is_rmb_down && !game_state.mouse_state.mouse_on_ui) {
		// if there was no timer, then RMB was just pressed, create a new timer
		// and allow the swap action right away by setting progress to 100%
		if (rmb_timer == nullptr) {
			rmb_timer = new Timer(MOUSE_BUTTON_DELAY);
			rmb_timer->set_progress(1);
		}

		// if the timer is done counting, allow the RMB action and reset the timer
		if (rmb_timer->is_done()) {
			swap_balls();

			rmb_timer->reset(true);
		}
	}
	// if the RMB is not pressed down, remove the delay timer to set up for the next RMB press
	else {
		if (rmb_timer && rmb_timer->is_done()) {
			delete rmb_timer;
			rmb_timer = nullptr;
		}
	}

	// if there's an animation going on, transform the player locally based on animation's progress
	if (current_animation != nullptr) {
		// if the animation is finished, remove it
		if (current_animation->is_finished()) {
			delete current_animation;
			current_animation = nullptr;
		}
		// otherwise, progress it
		else {
			float anim_progress = current_animation->get_progress();

			float shift = static_cast<float>(PLAYER_ANIM_SHIFT) * (1 - anim_progress);
			local_transform.position.y = -shift;
			secondary_drawing_ball->local_transform.position.y = -shift;
		}
	}

	// same with the drawing ball
	if (drawing_ball_animation != nullptr) {
		if (drawing_ball_animation->is_finished()) {
			delete drawing_ball_animation;
			drawing_ball_animation = nullptr;
		}
		else {
			float anim_progress = drawing_ball_animation->get_progress();

			float shift = static_cast<float>(PLAYER_ANIM_SHIFT * 2) * (1 - anim_progress);
			drawing_ball->local_transform.position.y = -shift;
		}
	}

	// update the "holding" ball to make sure it's animation frame is set
	drawing_ball->update(delta, game_state);
	secondary_drawing_ball->update(delta, game_state);

	// update the mouse button delay timers if they exist
	if (lmb_timer)
		lmb_timer->update(delta, game_state);
	if (rmb_timer)
		rmb_timer->update(delta, game_state);
}

void PlayerBall::set_insertion_animation(const uint& ball_segment_index, bool inserting_at_end) {
	velocity = 0;
	collision_enabled = false;

	insertion_animation = 
		new Animation(
			BallTrack::BALL_INSERTION_TIME, 
			TIMING_FUNCTIONS.at(TimingFunctionType::EaseOut)
		);
	const TrackSegment& ts = ball_track->get_track_segment_by_bs_index(static_cast<float>(ball_segment_index));

	saved_angle = global_transform.rotation;
	global_transform.rotation = 0;
	local_transform.rotation = saved_angle;
	target_angle = normalize_angle(ts.angle - saved_angle + 90) - 180;
	target_ball_segment_index = ball_segment_index;
	is_inserting_at_end = inserting_at_end;
}

void PlayerBall::update(const float& delta, GameState& game_state) {
	// update the inherited Ball
	Ball::update(delta, game_state);
	// update position according to velocity
	global_transform.position += velocity * delta;

	if (
		global_transform.position.x > WINDOW_WIDTH + Ball::BALL_SIZE ||
		global_transform.position.x < -static_cast<int>(Ball::BALL_SIZE) ||
		global_transform.position.y > WINDOW_HEIGHT + Ball::BALL_SIZE ||
		global_transform.position.y < -static_cast<int>(Ball::BALL_SIZE)
	) {
		entity_manager->schedule_to_delete(this);
		return;
	}

	if (collision_enabled) {
		auto collision_data = ball_track->get_collision_data(global_transform.position, 2);
		if (collision_data) {
			uint hit_ball_index =
				static_cast<uint>(
					ceilf(collision_data->ball_segment_position / Ball::BALL_SIZE)
				);


			auto& ball_segment = ball_track->ball_segments[collision_data->ball_segment_index];

			ball_track->add_insertion_space(collision_data->ball_segment_index, collision_data->ball_segment_position);

			set_insertion_animation(collision_data->ball_segment_index, hit_ball_index == 0);

			SoundManager::play_sound(asset_manager->get_audio("ball_collision_pitched"));
		}
	}

	if (insertion_animation) {
		if (insertion_animation->is_finished()) {
			delete insertion_animation;
			insertion_animation = nullptr;

			ball_track->insert_new_ball(target_ball_segment_index, color, is_inserting_at_end);

			entity_manager->schedule_to_delete(this);
		}
		else {
			float progress = insertion_animation->get_progress();

			local_transform.rotation = saved_angle + target_angle * progress;

			vec2 target_position =
				ball_track->get_insertion_pos_by_bs_index(static_cast<float>(target_ball_segment_index), is_inserting_at_end) - global_transform.position;

			local_transform.position = target_position * progress;

			insertion_animation->update(delta);
		}
	}
}

void PlayerBall::shoot(const float& angle) {
	velocity.x = cosf(deg_to_rad(angle)) * BALL_SPEED;
	velocity.y = sinf(deg_to_rad(angle)) * BALL_SPEED;
}
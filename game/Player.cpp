#include "Player.h"

Player::Player(
	const Texture& normal_texture,
	const Texture& action_texture,
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
	BallColor primary_color = (BallColor)(rand() % BALL_COLOR_COUNT);
	BallColor secondary_color = (BallColor)(rand() % BALL_COLOR_COUNT);

	primary_ball = 
		entity_manager->add_entity_raw(
			make_shared<Ball>(asset_manager, primary_color, vec2(0, 100))
		);
	primary_ball->origin_transform = &global_transform;

	secondary_ball = 
		entity_manager->add_entity_raw(
			make_shared<Ball>(asset_manager, secondary_color, vec2(0, 100))
		);
	secondary_ball->origin_transform = &global_transform;
	
	vertical_alignment = VerticalAlignment::Middle;
	horizontal_alignment = HorizontalAlignment::Center;
}

void Player::shoot_ball() {
	// create the shooting ball
	auto shooting_ball =
		entity_manager->add_entity_raw(
			make_shared<PlayerBall>(asset_manager, ball_track, primary_ball->color)
		);

	// copy all transforms from primary_ball to shooting_ball
	shooting_ball->global_transform = primary_ball->global_transform;
	shooting_ball->local_transform = primary_ball->local_transform;
	shooting_ball->origin_transform = primary_ball->origin_transform;

	// "flattens" origin transform to global transform
	shooting_ball->apply_origin_transform();
	// shoot the newly created PlayerBall
	shooting_ball->shoot(global_transform.rotation + 90);

	// "shift" balls forward and assign a new color to the secondary ball

	// swap balls places
	primary_ball.swap(secondary_ball);

	// create a new color and assign it to secondary_ball
	BallColor new_color = (BallColor)(rand() % BALL_COLOR_COUNT);
	secondary_ball->color = new_color;
}

void Player::draw(SDL_Renderer* renderer, const RendererState& renderer_state) const {
	primary_ball->draw(renderer, renderer_state);

	Sprite::draw(renderer, renderer_state);
}


void Player::update(const float& delta, GameState& game_state) {
	// if there's an animation in progress, update it
	if (current_animation != nullptr)
		current_animation->update(delta);

	// calculate player's rotation using trigonometry
	vec2 size = get_size();
	float w = (global_transform.position.x) - game_state.mouse_state.mouse_pos.x;
	float h = (global_transform.position.y) - game_state.mouse_state.mouse_pos.y;
	global_transform.rotation = rad_to_deg(atan(h / w)) + 90;
	// flip the angle if W was negative, as cmath's atan doesn't return negative values
	if (w < 0) global_transform.rotation += 180;

	// if the LMB is pressed down, sping back the player and shoot the ball
	if (game_state.mouse_state.is_lmb_pressed) {
		// if the texture isn't already the action one, change it
		if (*get_texture() != action_texture) change_texture(&action_texture);
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

		// if there was no timer, then LMB was just pressed, create a new timer
		// and allow the swap action right away by setting progress to 100%
		if (lmb_timer == nullptr) {
			lmb_timer = new Timer(MOUSE_BUTTON_DELAY);
			lmb_timer->set_progress(1);
		}

		if (lmb_timer->is_done()) {
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
	if (game_state.mouse_state.is_rmb_pressed) {
		// if there was no timer, then RMB was just pressed, create a new timer
		// and allow the swap action right away by setting progress to 100%
		if (rmb_timer == nullptr) {
			rmb_timer = new Timer(MOUSE_BUTTON_DELAY);
			rmb_timer->set_progress(1);
		}

		if (rmb_timer->is_done()) {
			primary_ball.swap(secondary_ball);
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
		}
	}

	// update the "holding" balls to make sure their animation frame is set
	primary_ball->update(delta, game_state);
	secondary_ball->update(delta, game_state);

	// update the mouse button delay timers if they exist
	if (lmb_timer)
		lmb_timer->update(delta, game_state);
	if (rmb_timer)
		rmb_timer->update(delta, game_state);
}

void PlayerBall::update(const float& delta, GameState& game_state) {
	// update the inherited Ball
	Ball::update(delta, game_state);
	// update position according to velocity
	global_transform.position += velocity * delta;

	auto collision_data = ball_track->get_collision_data(global_transform.position, 10);
	if (collision_data) {
		velocity = 0;
	}
}

void PlayerBall::shoot(const float& angle) {
	velocity.x = cosf(deg_to_rad(angle)) * static_cast<float>(BALL_SPEED);
	velocity.y = sinf(deg_to_rad(angle)) * static_cast<float>(BALL_SPEED);
}
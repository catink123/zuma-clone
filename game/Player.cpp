#include "../game/Player.h"

Player::Player(
	pair<Texture*, Texture*> player_textures, 
	GameState* game_state
) : 
	Sprite(player_textures.first, vec2(10, 10)), 
	normal_texture(player_textures.first), 
	action_texture(player_textures.second), 
	game_state(game_state)
{
	primary_ball = (BallColor)(rand() % BALL_COLOR_COUNT);
	secondary_ball = (BallColor)(rand() % BALL_COLOR_COUNT);
	vertical_alignment = VerticalAlignment::Middle;
	horizontal_alignment = HorizontalAlignment::Center;
}

void Player::draw(SDL_Renderer* renderer, const RendererState& renderer_state) {
	vec2 size = get_size();
	float w = (global_transform.position.x) - game_state->mouse_state.mouse_pos.x;
	float h = (global_transform.position.y) - game_state->mouse_state.mouse_pos.y;
	global_transform.rotation = rad_to_deg(atan(h / w)) + 90;
	if (w < 0) global_transform.rotation += 180;

	if (game_state->mouse_state.is_lmb_pressed) {
		if (get_texture() != action_texture) change_texture(action_texture);
		if (current_animation == nullptr) {
			current_animation =
				new Animation(
					1,
					TIMING_FUNCTIONS.at(TimingFunctionType::EaseOut)
				);
		}

		current_animation->set_progess(0);
	}
	else {
		if (get_texture() != normal_texture) change_texture(normal_texture);
	}

	if (current_animation != nullptr) {
		if (current_animation->is_finished()) {
			delete current_animation;
			current_animation = nullptr;
		}
		else {
			float angle_cos = cosf(deg_to_rad(global_transform.rotation - 90));
			float angle_sin = sinf(deg_to_rad(global_transform.rotation - 90));

			float anim_progress = current_animation->get_progress();

			float shift = static_cast<float>(PLAYER_ANIM_SHIFT) * (1 - anim_progress);
			local_transform.position.x = angle_cos * shift;
			local_transform.position.y = angle_sin * shift;
		}
	}

	Sprite::draw(renderer, renderer_state);
}

void Player::update(const float& delta) {
	if (current_animation == nullptr) return;
	current_animation->update(delta);
}
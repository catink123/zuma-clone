#include "Player.h"

Player::Player(
	pair<Texture*, Texture*> player_textures, 
	GameState* game_state
) : Sprite(player_textures.first, vec2(10, 10)), normal_texture(player_textures.first), action_texture(player_textures.second), game_state(game_state) {
	primary_ball = (BallColor)(rand() % BALL_COLOR_COUNT);
	secondary_ball = (BallColor)(rand() % BALL_COLOR_COUNT);
	vertical_alignment = VerticalAlignment::Middle;
	horizontal_alignment = HorizontalAlignment::Center;
}

void Player::draw(SDL_Renderer* renderer, const RendererState& renderer_state) {
	vec2 size = get_size();
	float w = (position.x) - game_state->mouse_state.mouse_pos.x;
	float h = (position.y) - game_state->mouse_state.mouse_pos.y;
	rotation = rad_to_deg(atan(h / w)) + 90;
	if (w < 0) rotation += 180;

	if (game_state->mouse_state.is_lmb_pressed) {
		if (get_texture() != action_texture) change_texture(action_texture);
	}
	else {
		if (get_texture() != normal_texture) change_texture(normal_texture);
	}

	Sprite::draw(renderer, renderer_state);
}
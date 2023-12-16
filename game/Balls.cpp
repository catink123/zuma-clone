#include "Balls.h"

Ball::Ball(
	shared_ptr<AssetManager> asset_manager,
	BallColor color,
	vec2 position
)
	:
	Sprite(
		&asset_manager->get_texture(
			BALL_COLOR_TEXTURE_MAP.at(color)	// gets texture name by BallColor value
		),
		position
	),
	color(color),
	ball_angle(0),
	asset_manager(asset_manager)
{
	sheen_sprite = make_shared<Sprite>(&asset_manager->get_texture("ball_sheen"));
	sheen_sprite->set_display_size(vec2(BALL_SIZE, BALL_SIZE));
	sheen_sprite->vertical_alignment = Middle;
	sheen_sprite->horizontal_alignment = Center;

	set_display_size(vec2(BALL_SIZE, BALL_SIZE));
	// align the ball to the absolute center
	vertical_alignment = Middle;
	horizontal_alignment = Center;
}

void Ball::draw(SDL_Renderer* renderer, const RendererState& renderer_state) const {
	Sprite::draw(renderer, renderer_state);
	sheen_sprite->draw(renderer, renderer_state);
}

void Ball::update(const float&, GameState&) {
	// angle step for each animation spritesheet's frame
	float step = 360.0F / BALL_ROTATION_FRAME_COUNT;
	// frame index into the animation spritesheet
	uint ball_frame = static_cast<uint>(floor(get_ball_angle() / step));
	// as the spritesheet isn't a string of balls, we need to calculate
	// XY position on the spritesheet
	uint sheet_x = ball_frame / BALL_SPRITESHEET_H;
	uint sheet_y = ball_frame % BALL_SPRITESHEET_H;

	// animation spritesheet's texture dimensions
	uint sheet_texture_w = get_texture()->get_width();
	uint sheet_texture_h = get_texture()->get_height();
	// dimensions of one animation frame
	uint frame_w = sheet_texture_w / (BALL_ROTATION_FRAME_COUNT / BALL_SPRITESHEET_H);
	uint frame_h = sheet_texture_h / BALL_SPRITESHEET_H;
	// calculation of clipping rectangle for the needed frame from the spritesheet
	SDL_Rect clip_rect;
	clip_rect.x = sheet_x * frame_w;
	clip_rect.y = sheet_y * frame_h;
	clip_rect.w = sheet_texture_w / (BALL_ROTATION_FRAME_COUNT / BALL_SPRITESHEET_H);
	clip_rect.h = sheet_texture_h / BALL_SPRITESHEET_H;
	// setting the clipping rectangle on the property inherited from Sprite class
	this->clip_rect = clip_rect;

	// set the ball sheen sprite transform to ball's transform, but reset the rotation
	sheen_sprite->global_transform = get_calculated_transform();
	sheen_sprite->global_transform.rotation = 0;
}

float Ball::get_ball_angle() const { return ball_angle; }
void Ball::set_ball_angle(const float& angle) {
	// sets the ball_angle to a angle in 0-360 format
	ball_angle = normalize_angle(angle);
}

void Ball::change_color(BallColor new_color) {
	color = new_color;
	change_texture(
		&asset_manager->get_texture(
			BALL_COLOR_TEXTURE_MAP.at(color)
		)
	);
}

uint BallSegment::get_total_length() const {
	return Ball::BALL_SIZE * static_cast<uint>(balls.size());
}

void BallSegment::shift() {
	is_shifting = true;

	if (shift_timer == nullptr)
		shift_timer = new Timer(BallTrack::BALL_INSERTION_TIME);

	shift_timer->reset(true);
}

BTCreationException::BTCreationException(const char* message) { msg = message; }
const char* BTCreationException::what() { return msg; }

BallTrack::BallTrack(
	const vector<vec2>& points, 
	const uint& ball_count, 
	shared_ptr<AssetManager> asset_manager, 
	shared_ptr<EntityManager> entity_manager
) : asset_manager(asset_manager), entity_manager(entity_manager) {
	// save the given points for later use
	cache.points = points;
	// if there aren't enough points to construct a track, throw an error
	if (points.size() < 2) {
		throw new BTCreationException("track point count is less than 2");
	}
	// construct track segments for each point pair
	for (int i = 1; i < points.size(); i++) {
		const vec2& point2 = points[i];
		const vec2& point1 = points[i - 1];
		TrackSegment segment;
		// using Pythagoras theorem to find the length, angle and cosine/sine of the angle of the segment
		float w = point2.x - point1.x;
		float h = point2.y - point1.y;
		segment.length = hypotf(w, h);
		segment.angle_cos = w / segment.length;
		segment.angle_sin = h / segment.length;
		segment.angle = acosf(segment.angle_cos);

		// as cmath's acos doesn't give negative values, we need to account for negative angles
		if (segment.angle_cos < 0)
			segment.angle -= static_cast<float>(M_PI);

		// if the height of the segment's right triangle is negative
		// (the segment's end point is closer to the top boundary than the start point),
		// the angle needs to be sign-inverted
		if (h < 0) { 
			segment.angle *= -1;
		}
		// if the width of the segment's right triangle is negative
		// (the segment's end point is closer to the left boundary than the start point),
		// the angle needs to be mirrored along the center point (add 180deg)
		if (w < 0) {
			segment.angle += static_cast<float>(M_PI);
		}
		// convert given angle to standard normalized degree form
		segment.angle = normalize_angle(rad_to_deg(segment.angle));

		cache.segments.push_back(segment);
	}

	// calculating and caching the total length of the track
	cache.total_length = 0.0F;
	for (TrackSegment segment : cache.segments) {
		cache.total_length += segment.length;
	}

	BallSegment segment;
	for (uint i = 0; i < ball_count; i++) {
		BallColor color = (BallColor)(rand() % BALL_COLOR_COUNT);
		segment.balls.push_back(
			Ball(asset_manager, color)
		);
	}
	ball_segments.push_back(segment);

	// shift back the created segment
	ball_segments[0].position = -(static_cast<float>(ball_segments[0].get_total_length()));

	// prepare the death hole sprite

	vec2 dh_pos = cache.points[0];

	for (const TrackSegment& ts : cache.segments) {
		dh_pos.x += ts.length * ts.angle_cos;
		dh_pos.y += ts.length * ts.angle_sin;
	}

	death_window = make_unique<Sprite>(&asset_manager->get_texture("death_window"), dh_pos, 0.25);
	death_window->horizontal_alignment = Center;
	death_window->vertical_alignment = Middle;
}

optional<uint> BallTrack::get_track_segment_by_position(const float& position) const {
	uint segment_index = 0;
	// go through the track segments and add up the total length from the start
	// until the given position is within the current total length, which means
	// that the given position is in the last added segment
	float current_length = cache.segments[0].length;
	while (current_length < position) {
		segment_index++;
		// if we hit the end, return the last segment's index
		if (segment_index >= cache.segments.size()) {
			return nullopt;
		}

		current_length += cache.segments[segment_index].length;
	}
	return segment_index;
}

vector<uint> BallTrack::get_track_segments_from_ball_segment(const BallSegment& ball_segment) const {
	vector<uint> result;

	// track segment index of the start of given ball segment
	optional<uint> start_track_index = get_track_segment_by_position(ball_segment.position);
	if (start_track_index == nullopt)
		start_track_index = 0;
	// track segment index of the end of given ball segment
	optional<uint> end_track_index = get_track_segment_by_position(ball_segment.position + ball_segment.get_total_length());
	if (end_track_index == nullopt)
		end_track_index = static_cast<uint>(cache.segments.size() - 1);

	// every track segment index between the start and end ones 
	// will also contain the given ball_segment, so we add the resulting range
	for (uint i = start_track_index.value(); i <= end_track_index.value(); i++)
		result.push_back(i);
	
	return result;
}

float BallTrack::get_track_segment_length_sum(const uint& last_segment) const {
	// if the given last segment index to sum to is more than there are segments, 
	// return the total length, as there won't be any more segments than currently exist
	if (last_segment >= cache.segments.size() - 1)
		return cache.total_length;

	float length = 0;
	for (int i = 0; i <= static_cast<int>(last_segment); i++)
		length += cache.segments[i].length;

	return length;
}

void BallTrack::draw(SDL_Renderer* renderer, const RendererState& renderer_state) const {
	death_window->draw(renderer, renderer_state);

	// go through each ball segment
	for (const BallSegment& segment : ball_segments)
		// and each of the balls of the segments
		for (const Ball& ball : segment.balls)
			// and draw the ball
			if (ball.show)
				ball.draw(renderer, renderer_state);

	for (const BallParticles& bp : ball_particles)
		bp.draw(renderer, renderer_state);
}

void BallTrack::update(const float& delta, GameState& game_state) {
	if (ball_segments.size() == 0) {
		if (!is_failing && !is_fading_out_to_screen) {
			is_fading_out_to_screen = true;
			game_state.fade_in([&]() {
				// delete Level to prepare for the next load
				entity_manager->schedule_to_delete("level");
				game_state.set_section(WinScreen);
				game_state.fade_out([](){}, 1.0F);
			}, 1.0F);
		}
	}

	// go through each ball segment
	for (int i = 0; i < ball_segments.size(); i++) {
		BallSegment& segment = ball_segments[i];

		if (segment.balls.size() == 0) {
			continue;
		}

		// and each of the balls of the segments
		for (int i = 0; i < segment.balls.size(); i++) {
			// calculate the current ball's position relative to the start of the track
			float ball_absolute_position = segment.position + i * Ball::BALL_SIZE;
			// find the track segment the ball is in
			optional<uint> track_segment_index = get_track_segment_by_position(ball_absolute_position);
			if (track_segment_index == nullopt) {
				segment.balls[i].show = false;
				continue;
			}
			else {
				segment.balls[i].show = true;
			}

			// fade out balls that are close to the death window

			float distance_to_end = cache.total_length - ball_absolute_position;
			if (segment.balls[i].show && distance_to_end < Ball::BALL_SIZE * 3)
				segment.balls[i].opacity = distance_to_end / (Ball::BALL_SIZE * 3);
			else
				segment.balls[i].opacity = 1;

			const TrackSegment& track_segment = cache.segments[track_segment_index.value()];

			// get the track's length up to the previous track segment...
			float total_sum = 0;
			if (track_segment_index.value() > 0)
				total_sum = get_track_segment_length_sum(track_segment_index.value() - 1);

			// ... to find the ball's position relative to the track segment it's in
			float ball_segment_position = ball_absolute_position - total_sum;

			Ball& ball = segment.balls[i];
			// rotate the ball along it's Z axis to point (or roll) in the direction of the track segment
			ball.global_transform.rotation = track_segment.angle + 90;
			// rotate the ball along it's local X axis (basically roll the ball) by the length of
			// the path it has rolled from the track's start
			ball.set_ball_angle(ball_absolute_position);

			vec2 previous_track_end_point = cache.points[track_segment_index.value()];

			const float& angle_cos = track_segment.angle_cos;
			const float& angle_sin = track_segment.angle_sin;

			// set the current ball's position along the track by offsetting it by 
			// the previous track end point position and adding it's "segment position" pointing
			// in the direction of the segment
			ball.global_transform.position.x = previous_track_end_point.x + angle_cos * ball_segment_position;
			ball.global_transform.position.y = previous_track_end_point.y + angle_sin * ball_segment_position;

			// and finally, update the ball after changing the ball angle
			ball.update(delta, game_state);
		}

		if (is_failing) {
			bool are_segments_left_on_screen = false;
			for (BallSegment& seg : ball_segments) {
				if (seg.position < cache.total_length) {
					are_segments_left_on_screen = true;
					break;
				}
			}
			if (are_segments_left_on_screen)
				speed_multiplier += FAIL_SEGMENT_ACCELERATION * delta;
			else if (!is_fading_out_to_screen) {
				is_fading_out_to_screen = true;

				game_state.fade_in([&]() {
					// delete Level to prepare for the next load
					entity_manager->schedule_to_delete("level");

					game_state.set_section(DeathScreen);
					game_state.fade_out([]() {}, 1.0F);
				}, 1.0F);
			}
		}

		if (segment.shift_timer) {
			if (segment.shift_timer->is_done()) {
				segment.is_shifting = false;

				segment.shift_timer->reset(true);
				float overrun = segment.shift_timer->get_current_time();
				segment.position -= (Ball::BALL_SIZE / BallTrack::BALL_INSERTION_TIME) * overrun;

				delete segment.shift_timer;
				segment.shift_timer = nullptr;
			}
			else {
				segment.shift_timer->update(delta, game_state);
			}
		}

		// move the segment by a calculated speed

		float total_speed = BASE_SPEED * speed_multiplier + segment.speed;

		if (segment.is_shifting)
			total_speed += Ball::BALL_SIZE / BallTrack::BALL_INSERTION_TIME;

		segment.position += delta * total_speed;

		// prevent any checks if death is in progress
		if (is_failing)
			continue;

		// check if the current segment collides with the next one

		// if we can select a next segment, check if it collides or
		// if the last ball of current segment is the same as the first of the next one
		if (i < ball_segments.size() - 1) {
			BallSegment& next_segment = ball_segments[i + 1];
			if (next_segment.balls.size() == 0)
				continue;

			bool adjacent_ball_same = segment.balls.back().color == next_segment.balls.front().color;
			if (adjacent_ball_same)
				next_segment.speed -= SEGMENT_FOLLOW_ACCELERATION * delta;

			// if the last ball of the current segment (position + length) touches the next segment with set error,
			// and the next segment is not shifting (i.e. a ball is being added into the segment) combine both ball segments into one
			if (segment.position + segment.get_total_length() >= next_segment.position + SEGMENT_COLLISION_ERROR && !next_segment.is_shifting) {
				connect_ball_segments(i);
				SoundManager::play_sound(asset_manager->get_audio("ball_collision"));
			}
		}

		// check if we have three or more balls of the same color in a row
		// and if so, break them

		BallColor saved_color = segment.balls[0].color;
		uint saved_ball_index = 0;
		uint same_color_count = 1;
		for (int i = 1; i < segment.balls.size(); i++) {
			BallColor current_color = segment.balls[i].color;
			if (saved_color == current_color)
				same_color_count++;
			else if (same_color_count >= 3)
				break;
			else {
				saved_color = current_color;
				saved_ball_index = i;
				same_color_count = 1;
			}
		}

		// if we found a string of 3 or more balls, break them
		if (same_color_count >= 3) {
			auto start_it = segment.balls.begin() + saved_ball_index;
			auto end_it = segment.balls.begin() + saved_ball_index + same_color_count;

			// add breaking particles
			for (uint i = saved_ball_index; i < saved_ball_index + same_color_count; i++) {
				const Ball& ball = segment.balls[i];

				ball_particles.push_back(
					move(
						BallParticles(
							ball.global_transform.position, 
							ball.color, 
							&asset_manager->get_texture("ball_particle")
						)
					)
				);
			}

			game_state.game_score += same_color_count * SCORE_PER_BALL;

			// delete the string of balls
			segment.balls.erase(start_it, end_it);
			
			// leave a blank space in place of them
			// if the blank space is not at the start, cut the segment
			if (saved_ball_index > 0)
				cut_ball_segment(i, static_cast<float>(saved_ball_index * Ball::BALL_SIZE), static_cast<float>(Ball::BALL_SIZE * same_color_count));
			// otherwise, just shift it by broken ball count
			else
				segment.position += Ball::BALL_SIZE * same_color_count;

			// play a breaking sound
			SoundManager::play_sound(asset_manager->get_audio("ball_break"));
		}

		// if some segment is out of the track length, we're dead
		if (segment.position + segment.get_total_length() > cache.total_length) {
			is_failing = true;
		}
	}

	// remove empty segments
	ball_segments.erase(
		remove_if(
			ball_segments.begin(),
			ball_segments.end(),
			[](BallSegment& seg) { return seg.balls.size() == 0; }
		),
		ball_segments.end()
	);

	for (int i = 0; i < ball_particles.size(); i++) {
		BallParticles& bp = ball_particles[i];
		bp.update(delta, game_state);

		if (bp.is_done())
			ball_particles.erase(ball_particles.begin() + i);
	}
}

optional<BallTrackCollisionData> BallTrack::get_collision_data(const vec2& point, const float& point_radius) const {
	BallTrackCollisionData collision_data;

	vec2* collision_point = nullptr;
	// go through all the track segments...
	for (int i = 0; i < cache.segments.size(); i++) {
		const TrackSegment& segment = cache.segments[i];
		vec2 start_point = cache.points[i];
		vec2 end_point = cache.points[i + 1];

		// ... and find if the given point with radius (circle) is on the track's line
		if (Collision::is_circle_on_line(start_point, end_point, point, point_radius)) {
			// the collision point will be the nearest point on the line
			vec2 p = Collision::get_closest_point_on_line(start_point, end_point, point);
			collision_point = &p;
			collision_data.track_segment_index = i;
			collision_data.track_segment_position = vec2(start_point, *collision_point).len();
			break;
		}
	}

	// if there was no collision with the track, then no collision happened at all
	if (collision_point == nullptr)
		return nullopt;

	// continue by going through all the ball segments... 
	for (int i = 0; i < ball_segments.size(); i++) {
		const BallSegment& ball_segment = ball_segments[i];
		// ... and seeing if any of the ball segments match the track segment, 
		// with which the given circle collided

		// get all track segment indecies that the ball segment goes through
		auto indecies = get_track_segments_from_ball_segment(ball_segment);
		// if the track index that corresponds to the possible collision is in the array of
		// indecies that the ball segment goes through, there could be a collision
		if (find(indecies.begin(), indecies.end(), collision_data.track_segment_index) != indecies.end()) {
			// get the collision position relative to the start of the track
			float absolute_collision_position = 
				get_track_segment_length_sum(collision_data.track_segment_index - 1) + collision_data.track_segment_position;

			// get the collision position relative to the start of the ball segment
			float ball_segment_collision_position = absolute_collision_position - ball_segment.position;
			// if the relative collision position is within the ball_segment's length, a collision happened
			if (
				ball_segment_collision_position >= -(static_cast<float>(Ball::BALL_SIZE) / 2) && 
				ball_segment_collision_position <= ball_segment.get_total_length() + (static_cast<float>(Ball::BALL_SIZE) / 2)
			) {
				collision_data.ball_segment_index = i;
				collision_data.ball_segment_position = ball_segment_collision_position;

				// return the constructed collision data early in the for-loop
				return collision_data;
			}
		}
		// otherwise there was no collision, check the next ball segment
	}
	
	// if we're here, the for-loop didn't return early, so no collision happened
	return nullopt;
}

bool BallTrack::cut_ball_segment(const uint& ball_segment_index, const float& position, const float& spacing) {
	// calculate the index of the last ball that will be in the first ball segment
	uint last_ball_index = static_cast<uint>(ceilf(position / Ball::BALL_SIZE));

	if (last_ball_index >= ball_segments[ball_segment_index].balls.size() || last_ball_index <= 0)
		return false;

	ball_segments.insert(ball_segments.begin() + ball_segment_index + 1, BallSegment());
	BallSegment& first_ball_segment = ball_segments[ball_segment_index];
	BallSegment& second_ball_segment = ball_segments[ball_segment_index + 1];

	// cut off balls before the hit one and add them to the new segment
	for (int i = last_ball_index; i < first_ball_segment.balls.size(); i++) {
		second_ball_segment.balls.push_back(first_ball_segment.balls[i]);
	}

	// cut off balls after the hit one
	first_ball_segment.balls.erase(first_ball_segment.balls.begin() + last_ball_index, first_ball_segment.balls.end() - 1);

	// remove one mystereously added ball at the end after the erase
	first_ball_segment.balls.pop_back();

	// calculate the position of the newly added ball_segment
	second_ball_segment.position = first_ball_segment.position + first_ball_segment.get_total_length() + spacing;

	return true;
}

void BallTrack::add_insertion_space(const uint& ball_segment_index, const float& position) {
	bool was_cut = cut_ball_segment(ball_segment_index, position);
	SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "inserting at: segment = %d; position %.2f; was_cut: %d\n", ball_segment_index, position, was_cut);

	// if the segment was cut, shift the second part
	if (was_cut)
		ball_segments[ball_segment_index + 1].shift();
}

void BallTrack::connect_ball_segments(const uint& ball_segment_index, bool inherit_seconds_speed) {
	// if the segment to connect is the last one, there is no
	// next one, return early in that case
	if (ball_segment_index >= ball_segments.size()) return;

	auto& first_ball_segment = ball_segments[ball_segment_index];
	auto& second_ball_segment = ball_segments[ball_segment_index + 1];

	if (inherit_seconds_speed)
		first_ball_segment.speed = second_ball_segment.speed;

	// move all balls from the second segment to the end of the first one
	first_ball_segment.balls.insert(
		first_ball_segment.balls.end(),
		make_move_iterator(second_ball_segment.balls.begin()),
		make_move_iterator(second_ball_segment.balls.end())
	);

	ball_segments.erase(ball_segments.begin() + ball_segment_index + 1);
}

const TrackSegment& BallTrack::get_track_segment_by_bs_index(const float& ball_segment_index) const {
	auto& ball_segment = ball_segments[static_cast<uint>(ball_segment_index)];

	optional<uint> track_segment_index = get_track_segment_by_position(
		ball_segment.position + ball_segment.get_total_length()
	);

	if (track_segment_index == nullopt)
		return cache.segments.back();
	else
		return cache.segments[track_segment_index.value()];
}

vec2 BallTrack::get_insertion_pos_by_bs_index(const float& ball_segment_index, bool inserting_at_end) const {
	auto& ball_segment = ball_segments[static_cast<uint>(ball_segment_index)];

	// calculate the ball's position relative to the start of the track
	float ball_absolute_position = ball_segment.position;
	if (!inserting_at_end)
		ball_absolute_position += ball_segment.get_total_length();

	// find the track segment the ball is in
	optional<uint> track_segment_index = get_track_segment_by_position(ball_absolute_position);
	if (track_segment_index == nullopt)
		track_segment_index = static_cast<uint>(cache.segments.size() - 1);

	const TrackSegment& track_segment = cache.segments[track_segment_index.value()];

	// get the track's length up to the previous track segment...
	float total_sum = 0;
	if (track_segment_index.value() > 0)
		total_sum = get_track_segment_length_sum(track_segment_index.value() - 1);

	// ... to find the ball's position relative to the track segment it's in
	float ball_segment_position = ball_absolute_position - total_sum;

	vec2 result = cache.points[0];

	// sum up global position for all track segments up to the previous one
	for (uint i = 0; i < track_segment_index.value(); i++) {
		const TrackSegment& current_track_segment = cache.segments[i];
		result.x += current_track_segment.length * current_track_segment.angle_cos;
		result.y += current_track_segment.length * current_track_segment.angle_sin;
	}

	// add the final track segment positoin
	result.x += ball_segment_position * track_segment.angle_cos;
	result.y += ball_segment_position * track_segment.angle_sin;

	if (inserting_at_end) {
		result.x -= Ball::BALL_SIZE * track_segment.angle_cos;
		result.y -= Ball::BALL_SIZE * track_segment.angle_sin;
	}

	return result;
}

void BallTrack::insert_new_ball(const uint& ball_segment_index, BallColor color, bool inserting_at_end) {
	BallSegment& segment = ball_segments[ball_segment_index];

	Ball new_ball(asset_manager, color);

	if (inserting_at_end) {
		segment.balls.insert(segment.balls.begin(), new_ball);
		segment.position -= Ball::BALL_SIZE;
	}
	else
		segment.balls.push_back(new_ball);
}

vector<BallColor> BallTrack::get_current_colors() const {
	vector<BallColor> result;
	for (const auto& segment : ball_segments) {
		for (const auto& ball : segment.balls) {
			if (find(result.begin(), result.end(), ball.color) == result.end())
				result.push_back(ball.color);
		}
	}
	return result;
}

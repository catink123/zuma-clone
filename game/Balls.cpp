#include "../game/Balls.h"

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
		shift_timer = new Timer(SHIFT_TIME);

	shift_timer->reset(true);
}

BTCreationException::BTCreationException(const char* message) { msg = message; }
const char* BTCreationException::what() { return msg; }

BallTrack::BallTrack(const vector<vec2>& points, shared_ptr<AssetManager> asset_manager) : asset_manager(asset_manager) {
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
	cache.total_length = 0;
	for (TrackSegment segment : cache.segments) {
		cache.total_length += segment.length;
	}

	BallSegment segment;
	for (int i = 0; i < 30; i++) {
		BallColor color = (BallColor)(rand() % BALL_COLOR_COUNT);
		segment.balls.push_back(
			Ball(asset_manager, color)
		);
	}
	ball_segments.push_back(segment);

	// temporary
	//ball_segments[0].position = -cache.total_length;
}

uint BallTrack::get_track_segment_by_position(const float& position) const {
	uint segment_index = 0;
	// go through the track segments and add up the total length from the start
	// until the given position is within the current total length, which means
	// that the given position is in the last added segment
	float current_length = cache.segments[0].length;
	while (current_length < position) {
		segment_index++;
		// if we hit the end, return the last segment's index
		if (segment_index >= cache.segments.size()) {
			return --segment_index;
		}

		current_length += cache.segments[segment_index].length;
	}
	return segment_index;
}

vector<uint> BallTrack::get_track_segments_from_ball_segment(const BallSegment& ball_segment) const {
	vector<uint> result;

	// track segment index of the start of given ball segment
	uint start_track_index = get_track_segment_by_position(ball_segment.position);
	// track segment index of the end of given ball segment
	uint end_track_index = get_track_segment_by_position(ball_segment.position + ball_segment.get_total_length());

	// every track segment index between the start and end ones 
	// will also contain the given ball_segment, so we add the resulting range
	for (int i = start_track_index; i <= end_track_index; i++)
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
	// go through each ball segment
	for (const BallSegment& segment : ball_segments)
		// and each of the balls of the segments
		for (const Ball& ball : segment.balls)
			// and draw the ball
			ball.draw(renderer, renderer_state);
}

void BallTrack::update(const float& delta, GameState& game_state) {
	// go through each ball segment
	for (BallSegment& segment : ball_segments) {
		// and each of the balls of the segments
		for (int i = 0; i < segment.balls.size(); i++) {
			// calculate the current ball's position relative to the start of the track
			float ball_absolute_position = segment.position + i * Ball::BALL_SIZE;
			// find the track segment the is in
			uint track_segment_index = get_track_segment_by_position(ball_absolute_position);
			const TrackSegment& track_segment = cache.segments[track_segment_index];

			// get the track's length up to the previous track segment...
			float total_sum = 0;
			if (track_segment_index > 0)
				total_sum = get_track_segment_length_sum(track_segment_index - 1);

			// ... to find the ball's position relative to the track segment it's in
			float ball_segment_position = ball_absolute_position - total_sum;

			Ball& ball = segment.balls[i];
			// rotate the ball along it's Z axis to point (or roll) in the direction of the track segment
			ball.global_transform.rotation = track_segment.angle + 90;
			// rotate the ball along it's local X axis (basically roll the ball) by the length of
			// the path it has rolled from the track's start
			ball.set_ball_angle(ball_absolute_position);

			vec2 previous_track_end_point = cache.points[track_segment_index];

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

		if (segment.shift_timer) {
			if (segment.shift_timer->is_done()) {
				segment.is_shifting = false;

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
			total_speed += Ball::BALL_SIZE / BallSegment::SHIFT_TIME;

		segment.position += delta * total_speed;
	}
}

optional<BallTrackCollisionData> BallTrack::get_collision_data(const vec2& point, const float& point_radius) const {
	BallTrackCollisionData collision_data;

	vec2* collision_point = nullptr;
	// go through all the track segments...
	for (int i = 0; i < cache.segments.size(); i++) {
		const TrackSegment& segment = cache.segments[i];
		const vec2& start_point = cache.points[i];
		const vec2& end_point = cache.points[i + 1];

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
			if (ball_segment_collision_position >= 0 && ball_segment_collision_position <= ball_segment.get_total_length()) {
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

bool BallTrack::cut_ball_segment(const uint& ball_segment_index, const float& position) {
	// calculate the index of the last ball that will be in the first ball segment
	uint last_ball_index = ceilf(position / Ball::BALL_SIZE);

	if (last_ball_index >= ball_segments[ball_segment_index].balls.size() || last_ball_index < 0)
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
	second_ball_segment.position = first_ball_segment.position + first_ball_segment.get_total_length();

	//second_ball_segment.speed += 20 + first_ball_segment.speed;
	second_ball_segment.shift();

	return true;
}

void BallTrack::insert_ball(const uint& ball_segment_index, const float& position, BallColor color) {
	// calculate the index of the last ball that will be in the first ball segment
	uint insertion_index = ceilf(position / Ball::BALL_SIZE);

	cut_ball_segment(ball_segment_index, position);

	// construct the new ball
	Ball new_ball(asset_manager, color);

	// add it the the first (if not the only) part
	//auto& first_part = ball_segments[ball_segment_index].balls;
	//first_part.insert(first_part.begin() + insertion_index, new_ball);
}

void BallTrack::connect_ball_segments(const uint& ball_segment_index) {
	// if the segment to connect is the last one, there is no
	// next one, return early in that case
	if (ball_segment_index >= ball_segments.size()) return;

	auto& first_ball_segment = ball_segments[ball_segment_index];
	auto& second_ball_segment = ball_segments[ball_segment_index + 1];

	// move all balls from the second segment to the end of the first one
	first_ball_segment.balls.insert(
		first_ball_segment.balls.end(),
		make_move_iterator(second_ball_segment.balls.begin()),
		make_move_iterator(second_ball_segment.balls.end())
	);

	ball_segments.erase(ball_segments.begin() + ball_segment_index + 1);
}

#include "../game/Balls.h"

Ball::Ball(
	shared_ptr<AssetManager> asset_manager, 
	BallColor color, 
	vec2 position
)
	:
	Sprite(
		asset_manager->get_texture(
			BALL_COLOR_TEXTURE_MAP.at(color)
		),
		position
	),
	color(color), ball_angle(0)
{
	set_display_size(vec2(BALL_SIZE, BALL_SIZE));
	vertical_alignment = Middle;
	horizontal_alignment = Center;
}

void Ball::draw(SDL_Renderer* renderer, const RendererState& renderer_state) {
	float step = 360.0F / BALL_ROTATION_FRAME_COUNT;
	uint ball_frame = static_cast<uint>(floor(ball_angle / step));
	uint sheet_x = ball_frame / BALL_SPRITESHEET_H;
	uint sheet_y = ball_frame % BALL_SPRITESHEET_H;

	uint sheet_texture_w = get_texture()->get_width();
	uint sheet_texture_h = get_texture()->get_height();
	uint frame_w = sheet_texture_w / (BALL_ROTATION_FRAME_COUNT / BALL_SPRITESHEET_H);
	uint frame_h = sheet_texture_h / BALL_SPRITESHEET_H;
	SDL_Rect* clip_rect = new SDL_Rect;
	clip_rect->x = sheet_x * frame_w;
	clip_rect->y = sheet_y * frame_h;
	clip_rect->w = sheet_texture_w / (BALL_ROTATION_FRAME_COUNT / BALL_SPRITESHEET_H);
	clip_rect->h = sheet_texture_h / BALL_SPRITESHEET_H;
	this->clip_rect = clip_rect;

	Sprite::draw(renderer, renderer_state);
	//set_ball_angle(get_ball_angle() + 0.1F);
}

float Ball::get_ball_angle() const { return ball_angle; }
void Ball::set_ball_angle(float angle) {
	ball_angle = normalize_angle(angle);
}

float BallSegment::get_total_length() const {
	return Ball::BALL_SIZE * balls.size();
}

BTCreationException::BTCreationException(const char* message) { msg = message; }
const char* BTCreationException::what() { return msg; }

BallTrack::BallTrack(const vector<vec2>& points, shared_ptr<AssetManager> asset_manager) : asset_manager(asset_manager) {
	cache.points = points;
	if (points.size() < 2) {
		throw new BTCreationException("track point count is less than 2");
	}
	for (int i = 1; i < points.size(); i++) {
		const vec2& point2 = points[i];
		const vec2& point1 = points[i - 1];
		TrackSegment segment;
		float w = point2.x - point1.x;
		float h = point2.y - point1.y;
		segment.length = hypotf(w, h);
		segment.angle_cos = w / segment.length;
		segment.angle_sin = h / segment.length;
		segment.angle = acosf(segment.angle_cos);

		if (h < 0) { 
			segment.angle *= -1;
		}
		if (w < 0) {
			segment.angle += static_cast<float>(M_PI);
		}
		segment.angle = normalize_angle(rad_to_deg(segment.angle));

		cache.segments.push_back(segment);
	}

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
}

int BallTrack::get_track_segment_by_position(const float& position) const {
	int segment_index = 0;
	float current_length = cache.segments[0].length;
	while (current_length < position) {
		segment_index++;
		if (segment_index >= cache.segments.size()) {
			return --segment_index;
		}

		current_length += cache.segments[segment_index].length;
	}
	return segment_index;
}

float BallTrack::get_track_segment_length_sum(const uint& last_segment) const {
	if (last_segment >= cache.segments.size() - 1)
		return cache.total_length;

	float length = 0;
	for (int i = 0; i <= last_segment; i++)
		length += cache.segments[i].length;

	return length;
}

void BallTrack::draw(SDL_Renderer* renderer, const RendererState& renderer_state) {
	for (BallSegment segment : ball_segments) {
		for (int i = 0; i < segment.balls.size(); i++) {
			float ball_absolute_position = segment.position + i * Ball::BALL_SIZE;
			float track_segment_index = get_track_segment_by_position(ball_absolute_position);
			const TrackSegment& track_segment = cache.segments[track_segment_index];

			float total_sum = 0;
			if (track_segment_index > 0)
				total_sum = get_track_segment_length_sum(track_segment_index - 1);

			float ball_segment_position = ball_absolute_position - total_sum;

			Ball& ball = segment.balls[i];
			ball.global_transform.rotation = track_segment.angle + 90;
			ball.set_ball_angle(ball_absolute_position);

			vec2 previous_track_end_point = cache.points[track_segment_index];

			//float angle_cos = cosf(deg_to_rad(track_segment.angle));
			//float angle_sin = sinf(deg_to_rad(track_segment.angle));
			const float& angle_cos = track_segment.angle_cos;
			const float& angle_sin = track_segment.angle_sin;

			ball.global_transform.position.x = previous_track_end_point.x + angle_cos * ball_segment_position;
			ball.global_transform.position.y = previous_track_end_point.y + angle_sin * ball_segment_position;

			ball.draw(renderer, renderer_state);
		}
	}
}

void BallTrack::update(const float& delta) {
	for (BallSegment& segment : ball_segments) {
		segment.position += delta * 10;
	}
}

bool BallTrack::is_point_on_balltrack(const vec2& point, const float& radius) const {
	for (int i = 0; i < cache.segments.size(); i++) {
		const TrackSegment& segment = cache.segments[i];
		const vec2& start_point = cache.points[i];
		const vec2& end_point = cache.points[i + 1];

		if (Collision::is_circle_on_line(start_point, end_point, point, Ball::BALL_SIZE + radius))
			return true;
	}

	return false;
}

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
	set_clip_rect(clip_rect);

	Sprite::draw(renderer, renderer_state);
	//set_ball_angle(get_ball_angle() + 0.1F);
}

float Ball::get_ball_angle() const { return ball_angle; }
void Ball::set_ball_angle(float angle) {
	ball_angle = normalize_angle(angle);
}

BTCreationException::BTCreationException(const char* message) { msg = message; }
const char* BTCreationException::what() { return msg; }

BallTrack::BallTrack(const vector<vec2>& points) {
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

		if (h < 0) segment.angle *= -1;
		segment.angle = normalize_angle(segment.angle);

		cache.segments.push_back(segment);
	}

	cache.total_length = 0;
	for (TrackSegment segment : cache.segments) {
		cache.total_length += segment.length;
	}
}
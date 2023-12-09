#pragma once
#include "../engine/Texture.h"
#include "../engine/common.h"
#include <vector>

using namespace std;

struct LevelData {
	Texture* background;
	vector<vec2> track_points;
	vec2 player_position;
	float track_speed_multiplier = 1;
};

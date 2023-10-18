#pragma once
#include "../engine/Texture.h"
#include <vector>

using namespace std;

struct Level {
	Texture* background;
	vector<vec2> track_points;
	vec2 player_position;
	bool is_double_ended;
};

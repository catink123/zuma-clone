#include <SDL.h>
#include <SDL_main.h>
#include "engine/Engine.h"
#include <pugixml.hpp>

int main(int, char**) {
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
	Engine* engine = new Engine();
	engine->run_loop();
	delete engine;
	IMG_Quit();
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL_image quit.\n");
	SDL_Quit();
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL quit.\n");
	TTF_Quit();
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL_ttf quit.\n");
	return 0;
}
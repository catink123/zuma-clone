#include "engine/Engine.h"

#ifdef NDEBUG
int WinMain() {
#else
int main() {
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
#endif
	Engine* engine = new Engine();
	engine->run_loop();
	delete engine;
	IMG_Quit();
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL_image quit.\n");
	SDL_Quit();
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL quit.\n");
	TTF_Quit();
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL_ttf quit.\n");
}
#include "engine/Engine.h"

#ifdef NDEBUG
int WinMain() {
#else
int main() {
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
#endif
	Engine engine;
	engine.run_loop();
}
#include "engine/Engine.h"

#ifdef NDEBUG
int WinMain() {
#else
int main() {
#endif
	Engine engine;
	engine.run_loop();
}
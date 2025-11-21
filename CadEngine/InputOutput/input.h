#pragma once
#include <SDL3/SDL.h>
#include <vector>


static class Input {
public:
	static SDL_FPoint mousePos;
	static SDL_FPoint rawMousePos;
	static SDL_FPoint mousePosDif;
	static std::vector<int> mouseStates;
	static std::vector<int> keyStates;
	static std::vector<int> wheelStates;

	static void controller();
	static bool initController();

};
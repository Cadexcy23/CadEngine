#pragma once
#include <SDL3/SDL.h>

static struct Engine {
	static bool quit;

	
	static bool initEngine(const char* title = "CadEngine", SDL_WindowFlags winFlags = NULL);
};
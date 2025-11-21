#pragma once
#include <SDL3/SDL.h>

static class Timer {

public:
	static Uint64 lastUpdateTime;
	static float deltaTime;
	static float deltaSeconds;
	static float updatesPS;
	static float framesPS;

	static void profileUpdate();

};
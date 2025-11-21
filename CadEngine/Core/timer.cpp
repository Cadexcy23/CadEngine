#include "engine.h"
#include "timer.h"
#include "logger.h"


Uint64 Timer::lastUpdateTime;
float Timer::deltaTime;
float Timer::deltaSeconds;
float Timer::updatesPS;
float Timer::framesPS;


void Timer::profileUpdate()
{
	//Compare last update to current time
	Uint64 current = SDL_GetPerformanceCounter();
	double elapsed = (current - lastUpdateTime) / static_cast<double>(SDL_GetPerformanceFrequency());
	lastUpdateTime = SDL_GetPerformanceCounter();

	//Update our variables
	updatesPS = 1.0 / elapsed;
	deltaTime = elapsed * 1000.0;

	//Convert delta time in seconds and clamp it
	deltaSeconds = deltaTime / 1000;
	if (deltaSeconds < 0)
		deltaSeconds = 0;
	else if (deltaSeconds > 1)
		deltaSeconds = 1;

	//Print debug info
	if (Engine::showFPS)
	{
		Logger::log(Logger::LogCategory::General, Logger::LogLevel::Debug,
			"UPS: %f DeltaM: %f DeltaS: %f", updatesPS, deltaTime, deltaSeconds);
	}
}

#include "engine.h"
#include "time.h"
#include "logger.h"


Uint64 Time::lastUpdateTime;
double Time::deltaTime;
float Time::deltaSeconds;
float Time::updatesPS;
float Time::framesPS;
std::vector<Time::timer> timers;


Time::timer::timer(double time, int count, std::function<void()> callback)
	: duration(time), count(count), callback(callback), current(0.0), finished(false)
{
}

void Time::timer::update()
{
	if (finished)
		return;

	current += Time::deltaTime;

	if (current >= duration)
	{
		callback();

		if (count == 0)
		{
			finished = true;
		}
		else
		{
			// preserve leftover time to prevent drift
			current -= duration;
			count--;
		}
	}
}

Time::timer* Time::createTimer(double time, int count, std::function<void()> callback)
{
	timers.push_back(timer(time, count, callback));
	return &timers.back();
}

void Time::updateTimers()
{
	for (int i = 0; i < (int)timers.size(); )
	{
		timer* t = &timers[i];
		t->update();

		if (t->isFinished())
		{
			// remove + delete
			timers.erase(timers.begin() + i);
			delete t;//check if it still exists?
		}
		else
		{
			i++;
		}
	}
}

void Time::update()
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

	//Update active timers
	updateTimers();

	//Print debug info
	if (Engine::showFPS)
	{
		Logger::log(Logger::LogCategory::General, Logger::LogLevel::Debug,
			"UPS: %f DeltaM: %f DeltaS: %f", updatesPS, deltaTime, deltaSeconds);
	}
}



#pragma once
#include <SDL3/SDL.h>
#include <functional>

static class Time {

public:
	static Uint64 lastUpdateTime;
	static double deltaTime;
	static float deltaSeconds;
	static float updatesPS;
	static float framesPS;

	class timer
	{
	public:
		timer(double time, int count, std::function<void()> callback);

		void update();
		bool isFinished() const { return finished; }
		void setCallback(std::function<void()> cb) { callback = std::move(cb); }
		double getCurrent() const { return current; }
		//add func to kill a timer

	private:
		double duration;
		double current;
		int count;
		std::function<void()> callback;
		bool finished;
	};


	static Time::timer* createTimer(double time, int count, std::function<void()> callback);
	static void updateTimers();
	static void update();

};
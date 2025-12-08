#include "engine.h"
#include <random>


bool Engine::quit = false;
bool Engine::showFPS = false;
Uint32 Engine::debugLevel = 0;
int Engine::engineState = STATE_DEFAULT;
std::random_device rd;
std::mt19937 gen(rd());


float Engine::randInRange(SDL_FPoint range)
{
	std::uniform_real_distribution<float> dist(range.x, range.y);
	return dist(gen);
}

bool initSDL()
{
	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		Logger::log(Logger::LogCategory::Engine, Logger::LogLevel::Error, "SDL could not initialize! SDL Error: %s", SDL_GetError());
		return false;
	}

	Logger::log(Logger::LogCategory::Engine, Logger::LogLevel::Trace, "SDL initialized!");
	return true;
}

bool Engine::initEngine(const char* title, SDL_WindowFlags winFlags)
{
	srand(time(NULL) * clock());

	Logger::init();
	initSDL();
	Renderer::initRenderer();
	Text::initFont();

	Input::initController();

	return true;
}

void Engine::update()
{
	Input::controller();
	Scene::updateObjects();
	Time::update();
	Renderer::updateScreenProperties();
}
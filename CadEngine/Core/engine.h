#pragma once
#include <SDL3/SDL.h>

//include all modules
#include "../Core/logger.h"
#include "../Core/time.h"
#include "../Graphics/renderer.h"
#include "../Graphics/text.h"
#include "../Graphics/texture.h"
#include "../InputOutput/input.h"
#include "../InputOutput/serialization.h"
#include "../Network/network.h"
#include "../Scene/asset.h"
#include "../Scene/object.h"
#include "../Scene/scene.h"
#include "../Sound/sound.h"


static class Engine {
public:
	static bool quit;
	static bool showFPS;
	static Uint32 debugLevel;
	static int engineState;
	enum engineStates {
		STATE_DEFAULT,
		STATE_PAUSE,
	};
	
	static float randInRange(SDL_FPoint range);
	static bool initEngine(const char* title = "CadEngine", SDL_WindowFlags winFlags = NULL);
	static void update();
};
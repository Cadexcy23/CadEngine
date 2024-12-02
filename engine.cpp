#include <stdio.h>
#include <string>
#include "engine.h"

//private
SDL_Point resolution = { 1920/2, 1080/2 };
SDL_Renderer* renderer;
SDL_Window* window;
//public
bool Engine::quit = false;









bool initSDL()
{
	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		return false;
	}
	//Set Vsync
	if (!SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1"))
	{
		printf("Warning: Linear texture filtering not enabled!");
	}
	//add more hints here


	printf("SDL initialized!\n");
	return true;
}

bool initWindow(const char* title = "CadEngine", SDL_WindowFlags flags = NULL)
{
	window = SDL_CreateWindow(title, resolution.x, resolution.y, flags);
	//SDL_Surface* icon = IMG_Load("Resource/icon.png"); //ADD IMG STUFF THEN WE CAN HAVE AN ICON
	//SDL_SetWindowIcon(gWindow, icon);
	//SDL_FreeSurface(icon);
	if (window == NULL)
	{
		printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}


	printf("Window initialized!\n");
	return true;
}

bool Engine::initEngine(const char* title, SDL_WindowFlags winFlags)
{
	initSDL();
	initWindow(title, winFlags);

	return true;
}
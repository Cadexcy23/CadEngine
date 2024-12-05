#include <stdio.h>
#include <string>
#include <random>
#include "engine.h"

//private
SDL_Renderer* renderer;
SDL_Window* window;
SDL_Event event;
std::vector<Engine::engineObject> activeObjects;
const bool* SDLKeyStates;
std::vector<bool> mouseWheel;
//add scale
//public
bool Engine::quit = false;
SDL_Point Engine::resolution = { 1920/2, 1080/2 };
std::vector<int> Engine::keyStates;
SDL_FPoint Engine::mousePos = { 0, 0 }; //add a second one for last possition? maybe make this a vector?
std::vector<int> Engine::mouseStates; // make enum for buttons?
//add mouse dragging info? last pos? or drag stat? bofa?
std::vector<int> Engine::wheelStates; // make enum for directions?

//TEMP ZONE
SDL_Surface* tempSurf;
SDL_Texture* tempTexture;
SDL_FRect imgRect = { 0.0, 0.0, 100.0, 100.0 };

//Mixing

//Controlling

void readMouse()
{
	//get mouse button state and position
	Uint32 mouseState = SDL_GetMouseState(&Engine::mousePos.x, &Engine::mousePos.y);

	//loop through each button on the mouse
	for (int i = 0; i < Engine::mouseStates.size(); i++)
	{
		//if the button is pressed
		if (mouseState & SDL_BUTTON_MASK(i+1))
		{
			//if it wasnt held down set it to just pressed
			if (Engine::mouseStates[i] == 0 || Engine::mouseStates[i] == 3)
				Engine::mouseStates[i] = 1;
			//if it was already pressed mark as held down
			else if (Engine::mouseStates[i] == 1 || Engine::mouseStates[i] == 2)
				Engine::mouseStates[i] = 2;
		}
		//if not and the button state was non zero
		else if (Engine::mouseStates[i])
		{
			//if it was held down mark as just released
			if (Engine::mouseStates[i] == 1 || Engine::mouseStates[i] == 2)
				Engine::mouseStates[i] = 3;
			//if already released mark as clear
			else
				Engine::mouseStates[i] = 0;
		}
	}

	//loop through each direction of the mouse wheel
	for (int i = 0; i < mouseWheel.size(); i++)
	{
		//if the key is pressed
		if (mouseWheel[i])
		{
			//if it wasnt held down set it to just pressed
			if (Engine::wheelStates[i] == 0 || Engine::wheelStates[i] == 3)
				Engine::wheelStates[i] = 1;
			//if it was already pressed mark as held down
			else if (Engine::wheelStates[i] == 1 || Engine::wheelStates[i] == 2)
				Engine::wheelStates[i] = 2;
		}
		//if not and the key state was non zero
		else if (Engine::wheelStates[i])
		{
			//if it was held down mark as just released
			if (Engine::wheelStates[i] == 1 || Engine::wheelStates[i] == 2)
				Engine::wheelStates[i] = 3;
			//if already released mark as clear
			else
				Engine::wheelStates[i] = 0;
		}
	}
	//reset mouseWheel
	std::fill(mouseWheel.begin(), mouseWheel.end(), false);
	
	
	//draws mouse buttons and their states for debugging
	//printf("Pos: %f, %f ", Engine::mousePos.x, Engine::mousePos.y);
	//for (int i = 0; i < Engine::mouseStates.size(); i++)
	//{
	//	if (Engine::mouseStates[i] != 0)
	//	{
	//		printf("Button %i: %i\n", i, Engine::mouseStates[i]);
	//	}
	//}

	//draws mouse wheel and their states for debugging
	//for (int i = 0; i < Engine::wheelStates.size(); i++)
	//{
	//	if (Engine::wheelStates[i] != 0)
	//	{
	//		printf("Direction %i: %i\n", i, Engine::wheelStates[i]);
	//	}
	//}
}

static void readKeyboard()
{
	//loop through each key on the keyboard
	for (int i = 0; i < Engine::keyStates.size(); i++)
	{
		//if the key is pressed
		if (SDLKeyStates[i])
		{
			//if it wasnt held down set it to just pressed
			if (Engine::keyStates[i] == 0 || Engine::keyStates[i] == 3)
				Engine::keyStates[i] = 1;
			//if it was already pressed mark as held down
			else if (Engine::keyStates[i] == 1 || Engine::keyStates[i] == 2)
				Engine::keyStates[i] = 2;
		}
		//if not and the key state was non zero
		else if (Engine::keyStates[i])
		{
			//if it was held down mark as just released
			if (Engine::keyStates[i] == 1 || Engine::keyStates[i] == 2)
				Engine::keyStates[i] = 3;
			//if already released mark as clear
			else
				Engine::keyStates[i] = 0;
		}
	}

	//draws keys and their states for debugging
	//for (int i = 0; i < Engine::keyStates.size(); i++)
	//{
	//	if (Engine::keyStates[i] != 0)
	//	{
	//		printf("%i: %i \n", i, Engine::keyStates[i]);
	//	}
	//}
}

void Engine::controller()
{
	while (SDL_PollEvent(&event)) {
		//printf("input detected: %i\n", event.type);
		switch (event.type) {
		case SDL_EVENT_QUIT:
			Engine::quit = true;
			break;
		case SDL_EVENT_MOUSE_WHEEL:
			if (event.wheel.y > 0)
				mouseWheel[0] = true;
			else
				mouseWheel[1] = true;
			break;
		}
	}

	//TEMP ZONE
	if(Engine::keyStates[SDL_SCANCODE_ESCAPE])
		Engine::quit = true;
	if (Engine::wheelStates[0])
		imgRect.y--;
	if (Engine::wheelStates[1])
		imgRect.y++;
	if (Engine::keyStates[SDL_SCANCODE_A])
		imgRect.x--;
	if (Engine::keyStates[SDL_SCANCODE_D])
		imgRect.x++;

	readMouse();
	readKeyboard();
}

//Rendering

void Engine::drawLine(SDL_FPoint start, SDL_FPoint end, SDL_Color color)
{
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_RenderLine(renderer, start.x, start.y, end.x, end.y);
}

void renderObjects()
{
	for (const auto& obj : activeObjects) {
		obj.draw();
	}
}

void Engine::draw()
{
	//Clear screen
	SDL_RenderClear(renderer);

	//TEMP ZONE
	SDL_RenderTexture(renderer, tempTexture, NULL, &imgRect);
	for ( auto& obj : activeObjects) {
		obj.pos = { float(rand() % resolution.x), float(rand() % resolution.y) };
	}
	


	//Render all entities
	renderObjects();

	//Update screen
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderPresent(renderer);
}

//Initialization

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
	window = SDL_CreateWindow(title, Engine::resolution.x, Engine::resolution.y, flags);
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

bool initRenderer() 
{
	//Create renderer
	renderer = SDL_CreateRenderer(window, NULL);
	if (renderer == NULL)
	{
		printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}
	//Initialize renderer color
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	printf("Renderer initialized!\n");
	return true;
}

//bool initImage()

//bool initMixer()

//bool initFont()

bool initController()
{
	//get a handle on the SDL key states array
	SDLKeyStates = SDL_GetKeyboardState(NULL);
	//resize our vector to be the same amount
	Engine::keyStates.resize(SDL_SCANCODE_COUNT);
	//set each key to 0
	std::fill(Engine::keyStates.begin(), Engine::keyStates.end(), 0);

	//resize our mouseStates vector to hold all out mouse buttons
	Engine::mouseStates.resize(5);
	//set each key to 0
	std::fill(Engine::mouseStates.begin(), Engine::mouseStates.end(), 0);

	//resize/set default mouseWheel/wheelStates for each direction
	mouseWheel.resize(4);
	std::fill(mouseWheel.begin(), mouseWheel.end(), false);
	Engine::wheelStates.resize(4);
	std::fill(Engine::wheelStates.begin(), Engine::wheelStates.end(), 0);


	return true;
}

bool Engine::initEngine(const char* title, SDL_WindowFlags winFlags)
{
	initSDL();
	initWindow(title, winFlags);
	initRenderer();

	initController();

	//TEMP SHIT
	tempSurf = IMG_Load("resource/test.png");
	tempTexture = SDL_CreateTextureFromSurface(renderer, tempSurf);
	for (int i = 0; i < 100; i++)
	{
		activeObjects.push_back(engineObject({ 0, 0 }, { Uint8(rand() % 255), Uint8(rand() % 255), Uint8(rand() % 255),  255 }));
	}



	return true;
}



#include <stdio.h>
#include <random>
#include "engine.h"

//private
const bool* SDLKeyStates;
std::vector<bool> mouseWheel;
SDL_Renderer* renderer;
SDL_Window* window;
SDL_Texture* screenTex;
SDL_Event event;
std::vector<std::pair<const char*, SDL_Texture*>> activeTextures;
std::vector<std::pair<const char*, TTF_Font*>> activeFonts;
bool objectsModified = true;
Uint64 lastUpdateTime;
float deltaTime;
float updatesPS;
float framesPS;
//public
bool Engine::quit = false;
bool Engine::showFPS = true;
bool Engine::showDebug = false;
int Engine::engineState = STATE_DEFAULT;
SDL_Point Engine::baseRes = { 1920, 1080 };
SDL_Point Engine::resolution = { 1920 / 2, 1080 / 2 };
SDL_FPoint Engine::mousePos = { 0, 0 };
std::vector<int> Engine::mouseStates;
std::vector<int> Engine::keyStates;
std::vector<int> Engine::wheelStates;
float Engine::deltaSeconds;
std::vector<std::shared_ptr<Engine::engineObject>> activeObjects;


//Mixing

//Controlling

void profileUpdate()
{
	//Compare last update to current time
	Uint64 current = SDL_GetPerformanceCounter();
	double elapsed = (current - lastUpdateTime) / static_cast<double>(SDL_GetPerformanceFrequency());
	lastUpdateTime = SDL_GetPerformanceCounter();

	//Update our variables
	updatesPS = 1.0 / elapsed;
	deltaTime = elapsed * 1000.0;

	//Create public delta time in seconds and clamp it
	Engine::deltaSeconds = deltaTime / 1000;
	if (Engine::deltaSeconds < 0)
		Engine::deltaSeconds = 0;
	else if (Engine::deltaSeconds > 1)
		Engine::deltaSeconds = 1;

	//Print debug info
	if (Engine::showFPS)
	{
		printf("UPS: %f ", updatesPS);
		printf("DeltaM: %f ", deltaTime);
		printf("DeltaS: %f\n", Engine::deltaSeconds);
	}
}

void readMouse()
{
	//get mouse button state and position
	Uint32 mouseState = SDL_GetMouseState(&Engine::mousePos.x, &Engine::mousePos.y);

	//loop through each button on the mouse
	for (int i = 0; i < Engine::mouseStates.size(); i++)
	{
		//if the button is pressed
		if (mouseState & SDL_BUTTON_MASK(i + 1))
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

void updateObjects()
{
	for (auto& obj : activeObjects) {
		obj->update();
	}
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

	readMouse();
	readKeyboard();

	updateObjects();

	profileUpdate();
}

//Rendering

bool Engine::toggleVsync()
{
	int syncState;
	SDL_GetRenderVSync(renderer, &syncState);
	return SDL_SetRenderVSync(renderer, !syncState);
}

TTF_Font* Engine::loadFont(const char* path, int size)
{
	//Check if we already have this font loaded
	for (const auto& font : activeFonts)
	{
		//If we already have the texture loaded just return it
		if (font.first == path)
			return font.second;
	}

	TTF_Font* font = TTF_OpenFont(path, size);
	if (font == NULL)
	{
		printf("Failed to load font! SDL_ttf Error: %s\n", SDL_GetError());
		return NULL;
	}

	//Add font to our list of loaded textures
	activeFonts.push_back({ path, font });

	return font;
}

SDL_Texture* Engine::loadText(const char* text, TTF_Font* font, SDL_Color color)
{
	//Check if we already have this text loaded
	for (const auto& tex : activeTextures)
	{
		//If we already have the text loaded just return it
		if (tex.first == text + int(font) + color.r + color.g + color.b + color.a) //add color to this aswell
			return tex.second; //MAYBE have this add a timer for when this was last used so we can clear it
	}

	//Load image at specified path
	SDL_Surface* loadedSurface = TTF_RenderText_Blended(font, text, NULL, color);
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", text, SDL_GetError());
		return NULL;
	}
	//Create texture from surface pixels
	SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
	if (newTexture == NULL)
	{
		printf("Unable to create texture from %s! SDL Error: %s\n", text, SDL_GetError());
		return NULL;
	}
	//Get rid of old loaded surface
	SDL_DestroySurface(loadedSurface);

	//Add tex to our list of loaded textures
	activeTextures.push_back({ text + int(font) + color.r + color.g + color.b + color.a, newTexture });

	return newTexture;
}

SDL_Texture* Engine::loadTex(const char* path) // add flag for target texture
{
	//Check if we already have this texture loaded
	for (const auto& tex : activeTextures)
	{
		//If we already have the texture loaded just return it
		if (tex.first == path)
			return tex.second;
	}

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path);
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path, SDL_GetError());
		return NULL;
	}
	//Create texture from surface pixels
	SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
	if (newTexture == NULL)
	{
		printf("Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError());
		return NULL;
	}
	//Get rid of old loaded surface
	SDL_DestroySurface(loadedSurface);

	//Add tex to our list of loaded textures
	activeTextures.push_back({ path, newTexture });

	return newTexture;
}

void Engine::drawLine(SDL_FPoint start, SDL_FPoint end, SDL_Color color)
{
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_RenderLine(renderer, start.x, start.y, end.x, end.y);
}

void Engine::drawRect(SDL_FRect rect, SDL_Color color, bool fill)
{
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	if (fill)
		SDL_RenderFillRect(renderer, &rect);
	else
		SDL_RenderRect(renderer, &rect);
}

void Engine::drawTex(SDL_Texture* tex, SDL_FRect rect, double rot, bool center, SDL_FlipMode flip, float scale)
{
	SDL_FRect newRect = rect;
	newRect.w *= scale;
	newRect.h *= scale;
	switch (center)
	{
	case true:
		newRect.x -= newRect.w / 2;
		newRect.y -= newRect.h / 2;
		SDL_RenderTextureRotated(renderer, tex, NULL, &newRect, rot, NULL, flip);
		break;
	case false:
		SDL_RenderTextureRotated(renderer, tex, NULL, &newRect, rot, NULL, flip);
		break;
	}
}

//Objects

std::shared_ptr<Engine::engineObject> Engine::registerObject(std::shared_ptr<Engine::engineObject> obj)//maybe pointer? myabe make this more of a register object
{
	objectsModified = true;
	activeObjects.push_back(obj);
	return activeObjects.back();
}

bool compDepth(std::shared_ptr<Engine::engineObject> a, std::shared_ptr<Engine::engineObject> b)
{
	return a->depth > b->depth;
}

void sortObjects()
{
	std::sort(activeObjects.begin(), activeObjects.end(), compDepth);
	//printf("Sorted\n");
}

void renderObjects()
{
	if (objectsModified)
	{
		sortObjects();
		objectsModified = false;
	}
	for (auto& obj : activeObjects) {
		obj->draw();
	}
}

void renderScreen() // would having a second renderer for just doing this part make faster?
{
	//set renderr to the real window again
	SDL_SetRenderTarget(renderer, NULL);
	//render screenTex to it SCALED to the actual window size
	SDL_RenderTexture(renderer, screenTex, NULL, NULL);
	//reset renderer to the screentex
	SDL_SetRenderTarget(renderer, screenTex);
}

void Engine::draw()
{
	//Clear screen
	SDL_RenderClear(renderer);

	//Render all entities
	renderObjects();

	//Run final render
	renderScreen();

	//Update screen
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

	printf("SDL initialized!\n");
	return true;
}

bool initWindow(const char* title = "CadEngine", SDL_WindowFlags flags = NULL)
{
	window = SDL_CreateWindow(title, Engine::resolution.x, Engine::resolution.y, flags);
	SDL_Surface* icon = IMG_Load("Resource/icon.png");
	SDL_SetWindowIcon(window, icon);
	SDL_DestroySurface(icon);
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

	//Set Vsync
	SDL_SetRenderVSync(renderer, 1);

	//Create screen texture and set it as our render texture
	screenTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, Engine::baseRes.x, Engine::baseRes.y);
	SDL_SetRenderTarget(renderer, screenTex);

	printf("Renderer initialized!\n");
	return true;
}

//bool initMixer()

bool initFont()
{
	//Initialize SDL_ttf
	if (TTF_Init() == -1)
	{
		printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", SDL_GetError());
		return false;
	}
}

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
	srand(time(NULL) * clock());
	initSDL();
	initWindow(title, winFlags);
	initRenderer();
	initFont();

	initController();

	return true;
}



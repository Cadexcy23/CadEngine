#include <stdio.h>
#include <random>
#include <string>
#include "engine.h"

//private
const bool* SDLKeyStates;
std::vector<bool> mouseWheel;
SDL_Renderer* renderer;
SDL_Window* window;
SDL_Texture* screenTex;
SDL_Event event;
std::vector<std::pair<std::string, SDL_Texture*>> activeTextures;
std::vector<std::pair<std::string, TTF_Font*>> activeFonts;
bool objectsModified = true;
Uint64 lastUpdateTime;
float deltaTime;
float updatesPS;
float framesPS;
//public
bool Engine::quit = false;
bool Engine::showFPS = false;
Uint32 Engine::debugLevel = 0;
int Engine::engineState = STATE_DEFAULT;
SDL_Point Engine::baseRes = { 1920 / 1, 1080 / 1 };
SDL_Point Engine::windowRes = { 1920 / 2, 1080 / 2 };
SDL_FPoint Engine::resScale = { float(Engine::baseRes.x) / float(Engine::windowRes.x), float(Engine::baseRes.y) / float(Engine::windowRes.y) };
SDL_ScaleMode Engine::scaleMode = SDL_SCALEMODE_LINEAR;
float Engine::zoom = 1;
SDL_FPoint Engine::camPos = { 0, 0 };
SDL_FRect Engine::screenBounds = { 0, 0, 1920 / 1, 1080 / 1 };
SDL_FPoint Engine::screenOffSet = { 0, 0 };
double Engine::screenRotation = 0;
SDL_FPoint Engine::mousePos = { 0, 0 };
SDL_FPoint Engine::rawMousePos = { 0, 0 };
std::vector<int> Engine::mouseStates;
std::vector<int> Engine::keyStates;
std::vector<int> Engine::wheelStates;
float Engine::deltaSeconds;
std::vector<std::shared_ptr<Engine::engineObject>> activeObjects;
std::vector<std::shared_ptr<Engine::engineObject>> addObjects;


//Mixing

//Rendering
SDL_Texture* Engine::setRenderTarget(SDL_Texture* tex)
{
	SDL_Texture* lastTex = SDL_GetRenderTarget(renderer);
	SDL_SetRenderTarget(renderer, tex);
	return lastTex;
}

SDL_Point Engine::setResolution(SDL_Point res)
{
	SDL_Point oldRes = windowRes;
	windowRes = res;
	SDL_SetWindowSize(window, windowRes.x, windowRes.y);
	Engine::resScale = { float(Engine::baseRes.x) / float(Engine::windowRes.x), float(Engine::baseRes.y) / float(Engine::windowRes.y) };
	return oldRes;
}

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
		if (tex.first == std::string(text) + std::to_string(int(font)) + std::to_string(color.r) + std::to_string(color.g) + std::to_string(color.b) + std::to_string(color.a))
			return tex.second;
	}

	//Load image at specified path
	SDL_Surface* loadedSurface = TTF_RenderText_Blended(font, text, NULL, color);
	if (loadedSurface == NULL)
	{
		printf("Unable to load text %s! SDL_image Error: %s\n", text, SDL_GetError());
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
	activeTextures.push_back({ std::string(text) + std::to_string(int(font)) + std::to_string(color.r) + std::to_string(color.g) + std::to_string(color.b) + std::to_string(color.a), newTexture });

	return newTexture;
}

SDL_Texture* Engine::loadTex(const char* path)
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

	//Set scale mode for texture
	SDL_SetTextureScaleMode(newTexture, scaleMode);

	//Add tex to our list of loaded textures
	activeTextures.push_back({ path, newTexture });

	return newTexture;
}

SDL_Texture* Engine::loadTargetTex(SDL_Point size)
{
	//Load texture with the target flag
	SDL_Texture* newTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, size.x, size.y);

	//Set scale mode for texture
	SDL_SetTextureScaleMode(newTexture, scaleMode);

	return newTexture;
}

void Engine::drawLine(SDL_FPoint start, SDL_FPoint end, SDL_Color color)
{
	Uint8 r, g, b, a;
	SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_RenderLine(renderer, start.x, start.y, end.x, end.y);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

void Engine::drawRect(SDL_FRect rect, SDL_Color color, bool fill, bool centered)
{
	Uint8 r, g, b, a;
	SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	if (centered)
	{
		rect.x -= rect.w / 2;
		rect.y -= rect.h / 2;
	}
	if (fill)
		SDL_RenderFillRect(renderer, &rect);
	else
		SDL_RenderRect(renderer, &rect);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

void Engine::drawTex(SDL_Texture* tex, SDL_FRect rect, double rot, bool center, SDL_FlipMode flip, float scale, SDL_FRect* chunk)
{
	SDL_FRect newRect = rect;
	newRect.w *= scale;
	newRect.h *= scale;
	switch (center)
	{
	case true:
		newRect.x -= newRect.w / 2;
		newRect.y -= newRect.h / 2;
		SDL_RenderTextureRotated(renderer, tex, chunk, &newRect, rot, NULL, flip);
		break;
	case false:
		SDL_RenderTextureRotated(renderer, tex, chunk, &newRect, rot, NULL, flip);
		break;
	}
}

//Objects
std::shared_ptr<Engine::engineObject> Engine::addObject(std::shared_ptr<Engine::engineObject> obj)//maybe pointer? myabe make this more of a register object
{
	addObjects.push_back(obj);
	return addObjects.back();
}

std::shared_ptr<Engine::engineObject> registerObject(std::shared_ptr<Engine::engineObject> obj)//maybe pointer? myabe make this more of a register object
{
	objectsModified = true;
	activeObjects.push_back(obj);
	return activeObjects.back();
}

void removeObject(std::shared_ptr<Engine::engineObject> obj)
{
	auto it = std::remove(activeObjects.begin(), activeObjects.end(), obj);
	activeObjects.erase(it, activeObjects.end());
}

void Engine::removeAllObjects()
{
	for (auto& obj : activeObjects) {
		obj->remove = true;
	}
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

void renderScreen()
{
	//set renderer to the real window again
	SDL_SetRenderTarget(renderer, NULL);
	//render screenTex scaled to the actual window size
	SDL_FRect screenRect = { Engine::screenOffSet.x, Engine::screenOffSet.y, Engine::windowRes.x, Engine::windowRes.y };
	SDL_RenderTextureRotated(renderer, screenTex, NULL, &screenRect, Engine::screenRotation, NULL, SDL_FLIP_NONE);
	//reset renderer to the screenTex
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

//Controlling
void updateScreenProperties()
{
	//Set the outer most edges of the map
	Engine::screenBounds.x = Engine::camPos.x - (Engine::baseRes.x / 2) / Engine::zoom;
	Engine::screenBounds.y = Engine::camPos.y - (Engine::baseRes.y / 2) / Engine::zoom;
	Engine::screenBounds.w = Engine::camPos.x + (Engine::baseRes.x / 2) / Engine::zoom;
	Engine::screenBounds.h = Engine::camPos.y + (Engine::baseRes.y / 2) / Engine::zoom;
}

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
	Uint32 mouseState = SDL_GetMouseState(&Engine::rawMousePos.x, &Engine::rawMousePos.y);
	Engine::rawMousePos.x *= Engine::resScale.x;
	Engine::rawMousePos.y *= Engine::resScale.y;
	Engine::mousePos.x = Engine::rawMousePos.x;
	Engine::mousePos.y = Engine::rawMousePos.y;
	//adjust mouse pos in relation to the camera pos and zoom
	Engine::mousePos.x -= float(Engine::baseRes.x / 2);
	Engine::mousePos.x /= Engine::zoom;
	Engine::mousePos.x += Engine::camPos.x;
	Engine::mousePos.y -= float(Engine::baseRes.y / 2);
	Engine::mousePos.y /= Engine::zoom;
	Engine::mousePos.y += Engine::camPos.y;

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
	std::vector<std::shared_ptr<Engine::engineObject>> remObjs = {};
	for (auto& obj : activeObjects) {
		obj->update();
		if (obj->remove)
			remObjs.push_back(obj);
	}
	while (remObjs.size() > 0)
	{
		removeObject(remObjs.back());
		remObjs.pop_back();
	}
	while (addObjects.size() > 0)
	{
		registerObject(addObjects.back());
		addObjects.pop_back();
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
	updateScreenProperties();
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
	window = SDL_CreateWindow(title, Engine::windowRes.x, Engine::windowRes.y, flags);
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
	//Initialize renderer blend mode
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	//Set Vsync
	SDL_SetRenderVSync(renderer, 1);

	//Create screen texture and set it as our render texture
	screenTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, Engine::baseRes.x, Engine::baseRes.y);
	SDL_SetRenderTarget(renderer, screenTex);

	//Set scale mode for texture
	SDL_SetTextureScaleMode(screenTex, Engine::scaleMode);

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



#include <stdio.h>
#include <random>
#include "engine.h"

//private
const bool* SDLKeyStates;
std::vector<bool> mouseWheel;
SDL_Renderer* renderer;
SDL_Window* window;
SDL_Texture* screenTex;
static SDL_Point baseRes = { 1920, 1080 };
SDL_Event event;
std::vector<Engine::engineObject> activeObjects;
std::vector<Engine::HUDElement> activeElements;
std::vector<std::pair<const char*, SDL_Texture*>> activeTextures;
std::vector<std::pair<const char*, TTF_Font*>> activeFonts;
Uint64 lastUpdateTime;
float deltaTime;
float updatesPS;
float framesPS;
//public
bool Engine::quit = false;
bool Engine::showFPS = false;
bool Engine::showDebug = false;
int Engine::engineState = STATE_DEFAULT;
SDL_Point Engine::resolution = { 1920/2, 1080/2 };
SDL_FPoint Engine::mousePos = { 0, 0 };
std::vector<int> Engine::mouseStates;
std::vector<int> Engine::keyStates;
std::vector<int> Engine::wheelStates;
float Engine::deltaSeconds;


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
	printf("UPS: %f ", updatesPS);
	printf("DeltaM: %f ", deltaTime);
	printf("DeltaS: %f\n", Engine::deltaSeconds);
}

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

void updateObjects()
{
	for (auto& obj : activeObjects) {
		obj.update();
	}
}

void updateElements()
{
	for (auto& ele : activeElements) {
		ele.update();
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

	//TEMP ZONE
	if(Engine::keyStates[SDL_SCANCODE_ESCAPE])
		Engine::quit = true;
	if (Engine::keyStates[SDL_SCANCODE_V] == 1)
	{
		int syncState;
		SDL_GetRenderVSync(renderer, &syncState);
		SDL_SetRenderVSync(renderer, !syncState);
	}
	if (Engine::keyStates[SDL_SCANCODE_P] == 1)
	{
		if (Engine::engineState != Engine::STATE_PAUSE)
			Engine::engineState = Engine::STATE_PAUSE;
		else
			Engine::engineState = Engine::STATE_DEFAULT;
	}


	readMouse();
	readKeyboard();

	updateObjects();
	updateElements();

	profileUpdate();
}

//Rendering

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
	if(fill)
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

bool compDepth(Engine::engineObject a, Engine::engineObject b)
{
	return a.depth > b.depth;
}

void sortObjects()
{
	std::sort(activeObjects.begin(), activeObjects.end(), compDepth);
}

void renderObjects()
{
	for (auto& obj : activeObjects) {
		obj.draw();
	}
}

void renderElements()
{
	for (auto& ele : activeElements) {
		ele.draw();
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
	renderElements();

	//TEMP ZONE
	

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
	screenTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, baseRes.x, baseRes.y);
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

//TEMP FUNCs
//ENGINE OBJECTS
void tempUpdateFunc(Engine::engineObject* ent)
{
	if (Engine::engineState != Engine::STATE_PAUSE)
	{
		ent->hull.x += ent->vel.x * Engine::deltaSeconds;
		ent->hull.y += ent->vel.y * Engine::deltaSeconds;
		ent->vel.x -= (ent->vel.x * Engine::deltaSeconds) / 2;
		ent->vel.y -= (ent->vel.y * Engine::deltaSeconds) / 2;

		ent->rot += ent->spin * Engine::deltaSeconds;
		ent->spin -= (ent->spin * Engine::deltaSeconds) / 2;
	}
}

void keepInScreen(Engine::engineObject* ent)
{
	int left = 0 + (ent->hull.w / 2) * ent->scale;
	int right = baseRes.x - (ent->hull.w / 2) * ent->scale;
	int up = 0 + (ent->hull.h / 2) * ent->scale;
	int down = baseRes.y - (ent->hull.h / 2) * ent->scale;
	if (ent->hull.x < left)
	{
		ent->hull.x = left;
		ent->vel.x *= -1;
	}
	else if (ent->hull.x > right)
	{
		ent->hull.x = right;
		ent->vel.x *= -1;
	}
	if (ent->hull.y < up)
	{
		ent->hull.y = up;
		ent->vel.y *= -1;
	}
	else if (ent->hull.y > down)
	{
		ent->hull.y = down;
		ent->vel.y *= -1;
	}
}

void keyboardControl(Engine::engineObject* ent)
{
	if (Engine::engineState != Engine::STATE_PAUSE)
	{
		if (Engine::wheelStates[0])
			ent->scale *= 1.05;
		if (Engine::wheelStates[1])
			ent->scale *= 0.95;
		if (Engine::keyStates[SDL_SCANCODE_W])
			ent->vel.y -= 100 * Engine::deltaSeconds;
		if (Engine::keyStates[SDL_SCANCODE_S])
			ent->vel.y += 100 * Engine::deltaSeconds;
		if (Engine::keyStates[SDL_SCANCODE_A])
			ent->vel.x -= 100 * Engine::deltaSeconds;
		if (Engine::keyStates[SDL_SCANCODE_D])
			ent->vel.x += 100 * Engine::deltaSeconds;
		if (Engine::keyStates[SDL_SCANCODE_Q])
			ent->spin -= 360 * Engine::deltaSeconds;
		if (Engine::keyStates[SDL_SCANCODE_E])
			ent->spin += 360 * Engine::deltaSeconds;
		if (Engine::keyStates[SDL_SCANCODE_SPACE] == 1)
		{
			ent->vel.x *= 2;
			ent->vel.y *= 2;
		}
	}
}
//HUD ELEMENTS
void testDrawFuncEle(Engine::HUDElement* ele)
{
	Engine::drawTex(ele->tex, { ele->hull.x - 100,ele->hull.y, ele->hull.w, ele->hull.h }, ele->rot, ele->centered, ele->flip, ele->scale);
	Engine::drawTex(ele->tex, { ele->hull.x + 100,ele->hull.y, ele->hull.w, ele->hull.h }, ele->rot, ele->centered, ele->flip, ele->scale);
	Engine::drawTex(ele->tex, { ele->hull.x,ele->hull.y - 100, ele->hull.w, ele->hull.h }, ele->rot, ele->centered, ele->flip, ele->scale);
	Engine::drawTex(ele->tex, { ele->hull.x,ele->hull.y + 100, ele->hull.w, ele->hull.h }, ele->rot, ele->centered, ele->flip, ele->scale);
}

void colorChange(Engine::HUDElement* ele)
{
	Uint8 r, g, b;
	SDL_GetTextureColorMod(ele->tex, &r, &g, &b);//this is BAD
	SDL_SetTextureColorMod(ele->tex, r + rand() % 2, g + rand() % 2, b + rand() % 2);
}
//END TEMP FUNCs

bool Engine::initEngine(const char* title, SDL_WindowFlags winFlags)
{
	srand(time(NULL) * clock());
	initSDL();
	initWindow(title, winFlags);
	initRenderer();
	initFont();

	initController();

	//TEMP ZONE
	activeObjects.push_back(engineObject({ 0, 0, 1920, 1080 }, loadTex("resource/bg.png"), 0, false));
	activeObjects[0].depth = 1;
	activeObjects.push_back(engineObject({ float(rand() % baseRes.x), float(rand() % baseRes.y) , 200, 200 }, loadTex("resource/icon.png")));
	activeObjects[1].depth = -1;
	//activeObjects[1].updateFuncs.push_back(keyboardControl);

	int startOffset = activeObjects.size();
	for (int i = 0; i < 1000; i++)
	{
		SDL_FRect hull = { float(rand() % baseRes.x), float(rand() % baseRes.y), 100, 100 };
		SDL_Texture* tex = loadTex("resource/test2.png");
		SDL_FPoint vel = { rand() % 1000 - 500, rand() % 1000 - 500 };
		double rot = rand() % 360;
		float scale = float(rand() % 1000) / 1000.0 + 0.5;
		double spin = double(rand() % 20000) / 1000.0 - 10.0;
		activeObjects.push_back(engineObject(hull, tex, rot, true, SDL_FLIP_NONE, scale, vel, spin));
	}
	sortObjects();
	for (auto& obj : activeObjects) {
		obj.updateFuncs.push_back(keepInScreen);
		obj.updateFuncs.push_back(tempUpdateFunc);
		obj.updateFuncs.push_back(keyboardControl);
	}
	activeObjects[0].updateFuncs.clear();

	loadFont("resource/font/segoeuithibd.ttf", 128);
	loadFont("resource/font/segoeuithisi.ttf", 128);

	SDL_Texture* tex = loadText("CadEngine", activeFonts[0].second, { 255, 255, 255, 255 });
	float w, h = 0;
	SDL_GetTextureSize(tex ,&w, &h);
	SDL_FRect hull = { 0, baseRes.y - h * 0.25, w, h };
	SDL_FPoint vel = { rand() % 1000 - 500, rand() % 1000 - 500 };
	activeElements.push_back(HUDElement(hull, tex));
	activeElements[0].scale = 0.25;
	activeElements[0].centered = false;
	//activeElements[0].drawFuncs.push_back(testDrawFuncEle);
	//activeElements[0].updateFuncs.push_back(colorChange);

	//SDL_Texture* texB = loadText("This is NOT a virus!", activeFonts[0].second, { 255, 0, 0, 255 });
	//float wB, hB = 0;
	//SDL_GetTextureSize(texB, &wB, &hB);
	//SDL_FRect hullB = { baseRes.x / 2, baseRes.y / 2, w, h };
	//SDL_FPoint velB = { rand() % 1000 - 500, rand() % 1000 - 500 };
	//activeElements.push_back(HUDElement(hullB, texB));
	//activeElements[1].hull.y += 100;
	//activeElements[1].scale = 0.75;
	//activeElements[1].drawFuncs.push_back(testDrawFuncEle);
	

	return true;
}



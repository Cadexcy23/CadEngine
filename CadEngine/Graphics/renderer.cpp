#include"renderer.h"
#include "../Core/Engine.h"
#include <regex>


SDL_Renderer* Renderer::renderer;
SDL_Window* Renderer::window;
SDL_Texture* Renderer::screenTex;
SDL_ScaleMode Renderer::scaleMode = SDL_SCALEMODE_LINEAR;
SDL_Point Renderer::baseRes = { 1920 / 1, 1080 / 1 };
SDL_FPoint Renderer::resScale = { float(Renderer::baseRes.x) / float(Renderer::windowRes.x), float(Renderer::baseRes.y) / float(Renderer::windowRes.y) };
SDL_Point Renderer::windowRes;
float Renderer::zoom = 1;
SDL_FPoint Renderer::camPos = { 0, 0 };
SDL_FRect Renderer::screenBounds = { 0, 0, baseRes.x, baseRes.y };
SDL_FPoint Renderer::screenOffSet = { 0, 0 };
double Renderer::screenRotation = 0;
bool Renderer::hullDebugDraw = false;


std::vector<std::string> splitString(const std::string& str, const std::string& delim = " ")
{
	std::vector<std::string> tokens;
	size_t prev = 0, pos = 0;
	do
	{
		pos = str.find(delim, prev);
		if (pos == std::string::npos) pos = str.length();
		std::string token = str.substr(prev, pos - prev);
		if (!token.empty()) tokens.push_back(token);
		prev = pos + delim.length();
	} while (pos < str.length() && prev < str.length());
	return tokens;
}

void Renderer::updateScreenProperties()
{
	//Set the outer most edges of the screen
	Renderer::screenBounds.x = Renderer::camPos.x - (Renderer::baseRes.x / 2) / Renderer::zoom;
	Renderer::screenBounds.y = Renderer::camPos.y - (Renderer::baseRes.y / 2) / Renderer::zoom;
	Renderer::screenBounds.w = Renderer::camPos.x + (Renderer::baseRes.x / 2) / Renderer::zoom;
	Renderer::screenBounds.h = Renderer::camPos.y + (Renderer::baseRes.y / 2) / Renderer::zoom;
}

SDL_Point Renderer::setResolution(SDL_Point res)
{
	SDL_Point oldRes = windowRes;
	windowRes = res;
	SDL_SetWindowSize(window, windowRes.x, windowRes.y);
	Renderer::resScale = { float(Renderer::baseRes.x) / float(Renderer::windowRes.x), float(Renderer::baseRes.y) / float(Renderer::windowRes.y) };
	return oldRes;
}

void Renderer::centerWindow()
{
	SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

bool Renderer::toggleVsync()
{
	int syncState;
	SDL_GetRenderVSync(renderer, &syncState);
	return SDL_SetRenderVSync(renderer, !syncState);
}

void Renderer::drawLine(SDL_FPoint start, SDL_FPoint end, SDL_Color color)
{
	Uint8 r, g, b, a;
	SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_RenderLine(renderer, start.x, start.y, end.x, end.y);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

void Renderer::drawRect(SDL_FRect rect, SDL_Color color, bool fill, bool centered)
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

void Renderer::renderScreen()
{
	//set renderer to the real window again
	SDL_SetRenderTarget(renderer, NULL);
	//render screenTex scaled to the actual window size
	SDL_FRect screenRect = { screenOffSet.x, screenOffSet.y, windowRes.x, Renderer::windowRes.y };
	SDL_RenderTextureRotated(renderer, screenTex, NULL, &screenRect, screenRotation, NULL, SDL_FLIP_NONE);
	//reset renderer to the screenTex
	SDL_SetRenderTarget(renderer, screenTex);
}

bool compDepth(std::shared_ptr<Object::engineObjectBase> a, std::shared_ptr<Object::engineObjectBase> b)
{
	return a->depth > b->depth;
}

void sortObjects()
{
	std::sort(Scene::activeObjects.begin(), Scene::activeObjects.end(), compDepth);
	Logger::log(Logger::LogCategory::Graphics, Logger::LogLevel::Trace, "Sorted");
}

void Renderer::renderObjects()
{
	if (Scene::objectsModified)
	{
		sortObjects();
		Scene::objectsModified = false;
	}
	for (auto& obj : Scene::activeObjects) {
		obj->draw();
	}
}

bool initWindow(const char* title, SDL_WindowFlags flags)
{
	//load settings from file
	std::string resString = Serialization::getSetting("Resolution");
	auto splitRes = splitString(resString);

	//apply aspect ratio
	SDL_Point res = { stoi(splitRes[0]), stoi(splitRes[1]) };
	Renderer::setResolution(res);

	Renderer::window = SDL_CreateWindow(title, res.x, res.y, flags);
	SDL_Surface* icon = IMG_Load("Resource/icon.png");
	SDL_SetWindowIcon(Renderer::window, icon);
	SDL_DestroySurface(icon);
	if (Renderer::window == NULL)
	{
		Logger::log(Logger::LogCategory::Graphics, Logger::LogLevel::Error, "Window could not be created! SDL Error: %s", SDL_GetError());
		return false;
	}

	Logger::log(Logger::LogCategory::Graphics, Logger::LogLevel::Trace, "Window initialized!");
	return true;
}

bool Renderer::initRenderer(const char* title, SDL_WindowFlags flags)
{
	if (!initWindow(title, flags))
		return false;

	//Create renderer
	renderer = SDL_CreateRenderer(window, NULL);
	if (renderer == NULL)
	{
		Logger::log(Logger::LogCategory::Graphics, Logger::LogLevel::Error, "Renderer could not be created! SDL Error: %s", SDL_GetError());
		return false;
	}
	//Initialize renderer color
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	//Initialize renderer blend mode
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	//Set Vsync
	int vsyncSetting = stoi(Serialization::getSetting("Vsync"));
	SDL_SetRenderVSync(renderer, vsyncSetting);

	//Create screen texture and set it as our render texture
	Renderer::screenTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, Renderer::baseRes.x, Renderer::baseRes.y);
	SDL_SetRenderTarget(renderer, screenTex);

	//Set scale mode for texture
	SDL_SetTextureScaleMode(screenTex, Renderer::scaleMode);

	Logger::log(Logger::LogCategory::Graphics, Logger::LogLevel::Trace, "Renderer initialized!");


	return true;
}

void Renderer::drawDebug()
{
	if (Engine::debugLevel >= 2)
	{
		Renderer::drawRect({ 0, 0, float(Renderer::baseRes.x * .8), 15 * 6 }, { 0, 0, 0, 255 / 2 }, true);

		float w, h;
		std::string camS = "CamX: " + std::to_string(Renderer::camPos.x) + " Y: " + std::to_string(Renderer::camPos.y);
		SDL_Texture* cam = Text::loadText(camS.c_str(), Text::loadFont("resource/font/segoeuithibd.ttf", 32), { 255, 255, 255, 255 });
		SDL_GetTextureSize(cam, &w, &h);
		h /= 3;
		Texture::drawTex(cam, { 0, h * 1, w / 3, h }, 0, false);

		std::string zoomS = "Zoom: " + std::to_string(Renderer::zoom);
		SDL_Texture* zoom = Text::loadText(zoomS.c_str(), Text::loadFont("resource/font/segoeuithibd.ttf", 32), { 255, 255, 255, 255 });
		SDL_GetTextureSize(zoom, &w, NULL);
		Texture::drawTex(zoom, { 0, h * 2, w / 3, h }, 0, false);

		std::string rouseS = "Rouse X: " + std::to_string(Input::rawMousePos.x) + " Y: " + std::to_string(Input::rawMousePos.y);;
		SDL_Texture* rouse = Text::loadText(rouseS.c_str(), Text::loadFont("resource/font/segoeuithibd.ttf", 32), { 255, 255, 255, 255 });
		SDL_GetTextureSize(rouse, &w, NULL);
		Texture::drawTex(rouse, { 0, h * 3, w / 3, h }, 0, false);

		std::string mouseS = "Mouse X: " + std::to_string(Input::mousePos.x) + " Y: " + std::to_string(Input::mousePos.y);;
		SDL_Texture* mouse = Text::loadText(mouseS.c_str(), Text::loadFont("resource/font/segoeuithibd.ttf", 32), { 255, 255, 255, 255 });
		SDL_GetTextureSize(mouse, &w, NULL);
		Texture::drawTex(mouse, { 0, h * 4, w / 3, h }, 0, false);

		std::string screenBoundsS = "Screen Bounds: L: " + std::to_string(Renderer::screenBounds.x) + " R: " + std::to_string(Renderer::screenBounds.w)
			+ " U: " + std::to_string(Renderer::screenBounds.y) + " D: " + std::to_string(Renderer::screenBounds.h);
		SDL_Texture* screenBounds = Text::loadText(screenBoundsS.c_str(), Text::loadFont("resource/font/segoeuithibd.ttf", 32), { 255, 255, 255, 255 });
		SDL_GetTextureSize(screenBounds, &w, NULL);
		Texture::drawTex(screenBounds, { 0, h * 5, w / 3, h }, 0, false);
	}
}

void Renderer::draw()
{
	//Clear screen
	SDL_RenderClear(renderer);

	//Render all entities
	Renderer::renderObjects();

	//Render Debug window
	drawDebug();

	//Run final render
	renderScreen();

	//Update screen
	SDL_RenderPresent(renderer);
}




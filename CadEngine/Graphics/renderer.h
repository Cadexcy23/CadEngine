#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_image.h>


static class Renderer {
public:
	static SDL_Renderer* renderer;
	static SDL_ScaleMode scaleMode;
	static SDL_Window* window;
	static SDL_Texture* screenTex;
	static SDL_Point baseRes;
	static SDL_Point windowRes;
	static SDL_FPoint resScale;
	static float zoom;
	static SDL_FPoint camPos;
	static SDL_FRect screenBounds;
	static SDL_FPoint screenOffSet;
	static double screenRotation;

	static void updateScreenProperties();
	static SDL_Point setResolution(SDL_Point res);
	static void centerWindow();
	static bool toggleVsync();
	static void drawLine(SDL_FPoint start, SDL_FPoint end, SDL_Color color = { 255, 255, 255, 255 });
	static void drawRect(SDL_FRect rect, SDL_Color color = { 255, 255, 255, 255 }, bool fill = false, bool centered = false);
	static void renderScreen();
	static void renderObjects();
	static bool initRenderer(const char* title = "CadEngine", SDL_WindowFlags flags = NULL);
	static void drawDebug();
	static void draw();

};
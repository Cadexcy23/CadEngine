#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_ttf.h>
#include <vector>
#include <string>

static class Text {

public:
	static std::vector<std::pair<std::string, TTF_Font*>> activeFonts;

	static TTF_Font* loadFont(const char* path, int size);
	static SDL_Texture* loadText(const char* text, TTF_Font* font, SDL_Color color);
	static bool initFont();

};
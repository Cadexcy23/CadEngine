#pragma once
#include <SDL3/SDL.h>
#include <vector>
#include <string>

static class Texture {
public:
	static std::vector<std::pair<std::string, SDL_Texture*>> activeTextures;

	static void clearTarget(SDL_Color color = { 0,0,0,255 });
	static SDL_Texture* setRenderTarget(SDL_Texture* tex);
	static SDL_Texture* clearTextureChunk(SDL_Texture* tex, SDL_FRect chunk = {});
	static void cleanActiveTextures();
	static SDL_Texture* loadTex(const char* file, bool unique = false);
	static SDL_Texture* loadTargetTex(SDL_Point size);
	static void drawTex(SDL_Texture* tex, SDL_FRect rect, double rot = 0.0, bool center = true, SDL_FlipMode flip = SDL_FLIP_NONE, float scale = 1.0, SDL_FRect* chunk = NULL);

};
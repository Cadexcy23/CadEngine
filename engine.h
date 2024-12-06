#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_image.h>
#include <vector>

static struct Engine {
	static bool quit;
	static SDL_Point resolution;
	static std::vector<int> keyStates;
	static SDL_FPoint mousePos;
	static std::vector<int> mouseStates;
	static std::vector<int> wheelStates;
	
	static bool initEngine(const char* title = "CadEngine", SDL_WindowFlags winFlags = NULL);
	static void controller();
	static SDL_Texture* loadTex(const char* file);
	static void drawLine(SDL_FPoint start, SDL_FPoint end, SDL_Color color = { 255, 255, 255, 255 }); //need?
	static void drawTex(SDL_Texture* tex, SDL_FRect rect);
	static void draw();


	struct engineObject {
		SDL_FRect rect;
		SDL_Texture* tex; //make this a vector/atlas for animation/states
		float* x = &rect.x;
		float* y = &rect.y;
		float* w = &rect.w;
		float* h = &rect.h;
		//layer (sort the vector of these by this b4 render)

		void draw() const
		{
			drawTex(tex, rect);
		}


		//vector of custom rendering functs

		engineObject(SDL_FPoint pos, SDL_FPoint size, SDL_Texture* tex) : rect({pos.x, pos.y, size.x, size.y}), tex(tex)
		{
			
		}
	};
};
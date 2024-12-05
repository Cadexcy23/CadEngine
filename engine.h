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
	static void drawLine(SDL_FPoint start, SDL_FPoint end, SDL_Color color = { 255, 255, 255, 255 }); //need?
	static void draw();

	

	struct engineObject {
		SDL_FPoint pos; //add a SDL_FRect and make these pointers to its x,y
		SDL_Color color;//TEMP
		//layer (sort the vector of these by this b4 render)

		void draw() const
		{
			drawLine({ pos.x - 2, pos.y - 2 }, { pos.x - 2, pos.y + 2 }, color);
			drawLine({ pos.x - 2, pos.y - 2 }, { pos.x + 2, pos.y - 2 }, color);
			drawLine({ pos.x + 2, pos.y - 2 }, { pos.x + 2, pos.y + 2 }, color);
			drawLine({ pos.x - 2, pos.y + 2 }, { pos.x + 2, pos.y + 2 }, color);
		}


		//vector of custom rendering functs

		engineObject(SDL_FPoint pos, SDL_Color color) : pos(pos), color(color)
		{
			
		}
	};
};
#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_image.h>
#include <vector>


static struct Engine {
	static bool quit;
	static bool showFPS;
	static SDL_Point resolution;
	static SDL_FPoint mousePos;
	static std::vector<int> mouseStates;
	static std::vector<int> keyStates;
	static std::vector<int> wheelStates;
	
	static bool initEngine(const char* title = "CadEngine", SDL_WindowFlags winFlags = NULL);
	static void controller();
	static SDL_Texture* loadTex(const char* file);
	static void drawLine(SDL_FPoint start, SDL_FPoint end, SDL_Color color = { 255, 255, 255, 255 });
	static void drawRect(SDL_FRect rect, SDL_Color color = { 255, 255, 255, 255 }, bool fill = true);
	static void drawTex(SDL_Texture* tex, SDL_FRect rect, double rot = 0.0, bool center = true, SDL_FlipMode flip = SDL_FLIP_NONE, float scale = 1.0);
	static void draw();


	struct engineObject {
		SDL_Texture* tex; //make this a vector/atlas for animation/states
		bool centered;
		SDL_FlipMode flip;
		float scale;
		SDL_FRect hull;
		SDL_FPoint vel;
		double rot;
		double spin;
		//layer (sort the vector of these by this b4 render)

		void draw() const //replace with a vector of attached draw funcs
		{
			drawTex(tex, hull, rot, centered, flip, scale);
		}
		void update()//replace with a vector of attached update funcs
		{
			hull.x += vel.x;
			hull.y += vel.y;
			vel.x -= vel.x / 1000;
			vel.y -= vel.y / 1000;

			rot += spin;
			spin -= spin / 1000;
		}


		//vector of custom rendering functs

		engineObject(const SDL_FRect& hull, SDL_Texture* tex, double rot = 0, bool centered = true, SDL_FlipMode flip = SDL_FLIP_NONE, float scale = 1.0, SDL_FPoint vel = {0, 0}, double spin = 0)
			: hull(hull), tex(tex), rot(rot), centered(centered), flip(flip), scale(scale), vel(vel), spin(spin){}
	};
};
#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_image.h>
#include <SDL3/SDL_ttf.h>
#include <vector>
#include <functional>


static struct Engine {
	static bool quit;
	static bool showFPS;
	static bool showDebug;
	static int engineState;
	enum engineStates {
		STATE_DEFAULT,
		STATE_PAUSE,
	};
	static SDL_Point resolution;
	static SDL_FPoint mousePos;
	static std::vector<int> mouseStates;
	static std::vector<int> keyStates;
	static std::vector<int> wheelStates;
	static float deltaSeconds;

	static bool initEngine(const char* title = "CadEngine", SDL_WindowFlags winFlags = NULL);
	static void controller();
	static TTF_Font* loadFont(const char* path, int size);
	static SDL_Texture* loadText(const char* text, TTF_Font* font, SDL_Color color);
	static SDL_Texture* loadTex(const char* file);
	static void drawLine(SDL_FPoint start, SDL_FPoint end, SDL_Color color = { 255, 255, 255, 255 });
	static void drawRect(SDL_FRect rect, SDL_Color color = { 255, 255, 255, 255 }, bool fill = true);
	static void drawTex(SDL_Texture* tex, SDL_FRect rect, double rot = 0.0, bool center = true, SDL_FlipMode flip = SDL_FLIP_NONE, float scale = 1.0);
	static void draw();


	struct engineObject {
		SDL_Texture* tex;
		bool centered;
		SDL_FlipMode flip;
		float scale;
		SDL_FRect hull;
		SDL_FPoint vel;// remove
		double rot;
		double spin;// remove
		int depth;
		bool drawDefault;
		bool drawFlag;
		bool updateFlag;
		std::vector<std::function<void(engineObject* ent)>> drawFuncs;
		std::vector<std::function<void(engineObject* ent)>> updateFuncs;

		void draw()
		{
			if(drawDefault)
				drawTex(tex, hull, rot, centered, flip, scale);
			for (auto& func : drawFuncs) {
				func(this);
			}
		}
		void update()
		{
			for (auto& func : updateFuncs) {
				func(this);
			}
		}

		engineObject(const SDL_FRect& hull, SDL_Texture* tex, double rot = 0,
			bool centered = true, SDL_FlipMode flip = SDL_FLIP_NONE, float scale = 1.0,
			SDL_FPoint vel = { 0, 0 }, double spin = 0, int depth = 0)
			: hull(hull), tex(tex), rot(rot),
			centered(centered), flip(flip), scale(scale),
			vel(vel), spin(spin), depth(depth),
			drawDefault(true), drawFlag(true), updateFlag(true) {}
	};

	struct HUDElement { //remove?
		SDL_Texture* tex;
		bool centered;
		SDL_FlipMode flip;
		float scale;
		SDL_FRect hull;
		double rot;
		int depth;
		bool drawDefault;
		bool drawFlag;
		bool updateFlag;
		std::vector<std::function<void(HUDElement* ent)>> drawFuncs;
		std::vector<std::function<void(HUDElement* ent)>> updateFuncs;

		void draw()
		{
			if (drawDefault)
				drawTex(tex, hull, rot, centered, flip, scale);
			for (auto& func : drawFuncs) {
				func(this);
			}
		}
		void update()
		{
			for (auto& func : updateFuncs) {
				func(this);
			}
		}

		HUDElement(const SDL_FRect& hull, SDL_Texture* tex, double rot = 0,
			bool centered = true, SDL_FlipMode flip = SDL_FLIP_NONE, float scale = 1.0,
			int depth = 0)
			: hull(hull), tex(tex), rot(rot),
			centered(centered), flip(flip), scale(scale),
			depth(depth),
			drawDefault(true), drawFlag(true), updateFlag(true) {}
	};
};
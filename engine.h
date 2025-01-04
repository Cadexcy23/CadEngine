#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_image.h>
#include <SDL3/SDL_ttf.h>
#include <vector>
#include <functional>
#include <memory>


static class Engine {
public:
	static bool quit;
	static bool showFPS;
	static bool showDebug;
	static int engineState;
	enum engineStates {
		STATE_DEFAULT,
		STATE_PAUSE,
	};
	static SDL_Point baseRes;
	static SDL_Point resolution;
	static SDL_FPoint mousePos;
	static std::vector<int> mouseStates;
	static std::vector<int> keyStates;
	static std::vector<int> wheelStates;
	static float deltaSeconds;

	static bool initEngine(const char* title = "CadEngine", SDL_WindowFlags winFlags = NULL);
	static void controller();
	static bool toggleVsync();
	static TTF_Font* loadFont(const char* path, int size);
	static SDL_Texture* loadText(const char* text, TTF_Font* font, SDL_Color color);
	static SDL_Texture* loadTex(const char* file);
	static void drawLine(SDL_FPoint start, SDL_FPoint end, SDL_Color color = { 255, 255, 255, 255 });
	static void drawRect(SDL_FRect rect, SDL_Color color = { 255, 255, 255, 255 }, bool fill = true);
	static void drawTex(SDL_Texture* tex, SDL_FRect rect, double rot = 0.0, bool center = true, SDL_FlipMode flip = SDL_FLIP_NONE, float scale = 1.0);
	static void draw();

	struct engineObject : public std::enable_shared_from_this<engineObject> {
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
		std::vector<std::function<void(std::shared_ptr<Engine::engineObject> obj)>> drawFuncs;
		std::vector<std::function<void(std::shared_ptr<Engine::engineObject> obj)>> updateFuncs;

		void draw()
		{
			if (drawDefault)
				drawTex(tex, hull, rot, centered, flip, scale);
			for (auto& func : drawFuncs) {
				func(shared_from_this());
			}
		}
		void update()
		{
			for (auto& func : updateFuncs) {
				func(shared_from_this());
			}
		}

		engineObject(const SDL_FRect& hull, SDL_Texture* tex, double rot = 0,
			bool centered = true, SDL_FlipMode flip = SDL_FLIP_NONE, float scale = 1.0,
			int depth = 0)
			: hull(hull), tex(tex), rot(rot),
			centered(centered), flip(flip), scale(scale),
			depth(depth),
			drawDefault(true), drawFlag(true), updateFlag(true) {}

		virtual ~engineObject() {}
	};



	static std::shared_ptr<Engine::engineObject> registerObject(std::shared_ptr<Engine::engineObject> obj);

};
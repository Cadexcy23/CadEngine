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
	static Uint32 debugLevel;
	static int engineState;
	enum engineStates {
		STATE_DEFAULT,
		STATE_PAUSE,
	};
	static SDL_Point baseRes;
	static SDL_Point windowRes;
	static SDL_FPoint resScale;
	static SDL_ScaleMode scaleMode;
	static SDL_FPoint mousePos;
	static std::vector<int> mouseStates;
	static std::vector<int> keyStates;
	static std::vector<int> wheelStates;
	static float deltaSeconds;

	static SDL_Texture* setRenderTarget(SDL_Texture* tex);
	static SDL_Point setResolution(SDL_Point res);
	static bool toggleVsync();
	static TTF_Font* loadFont(const char* path, int size);
	static SDL_Texture* loadText(const char* text, TTF_Font* font, SDL_Color color);
	static SDL_Texture* loadTex(const char* file);
	static SDL_Texture* loadTargetTex(SDL_Point size);
	static void drawLine(SDL_FPoint start, SDL_FPoint end, SDL_Color color = { 255, 255, 255, 255 });
	static void drawRect(SDL_FRect rect, SDL_Color color = { 255, 255, 255, 255 }, bool fill = true);
	static void drawTex(SDL_Texture* tex, SDL_FRect rect, double rot = 0.0, bool center = true, SDL_FlipMode flip = SDL_FLIP_NONE, float scale = 1.0, SDL_FRect* chunk = NULL);
	static void removeAllObjects();
	static void draw();
	static void controller();
	static bool initEngine(const char* title = "CadEngine", SDL_WindowFlags winFlags = NULL);

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
		bool remove;
		std::vector<std::function<void(std::shared_ptr<Engine::engineObject> obj)>> drawFuncs;
		std::vector<std::function<void(std::shared_ptr<Engine::engineObject> obj)>> updateFuncs;


		void drawHull()
		{
			SDL_FRect bounds = getBounds();

			Engine::drawLine({ bounds.x, bounds.y }, { bounds.w, bounds.y });
			Engine::drawLine({ bounds.w, bounds.y }, { bounds.w, bounds.h });
			Engine::drawLine({ bounds.w, bounds.h }, { bounds.x, bounds.h });
			Engine::drawLine({ bounds.x, bounds.h }, { bounds.x, bounds.y });

			Engine::drawLine({ bounds.x, bounds.y }, { bounds.w, bounds.h });
			Engine::drawLine({ bounds.x, bounds.h }, { bounds.w, bounds.y });
		}

		SDL_FRect getBounds()
		{
			if (centered)
			{
				return {
					hull.x - (hull.w * scale) / 2,
					hull.y - (hull.h * scale) / 2,
					hull.x + (hull.w * scale) - (hull.w * scale) / 2,
					hull.y + (hull.h * scale) - (hull.h * scale) / 2
				};
			}
			else
			{
				return {
					hull.x,
					hull.y,
					hull.x + (hull.w * scale),
					hull.y + (hull.h * scale)
				};
			}
		}

		bool mouseInBounds()
		{
			SDL_FRect bounds = getBounds();
			if (mousePos.x >= bounds.x && mousePos.x <= bounds.w && mousePos.y >= bounds.y && mousePos.y <= bounds.h)
			{
				return true;
			}
			return false;
		}

		void resetSize()
		{
			float w, h;
			SDL_GetTextureSize(tex, &w, &h);
			hull.w = w;
			hull.h = h;
			scale = 1;
		}

		virtual void draw()
		{
			if (drawFlag)
			{
				if (drawDefault)
					drawTex(tex, hull, rot, centered, flip, scale);
				if (debugLevel)
					drawHull();
				for (auto& func : drawFuncs) {
					func(shared_from_this());
				}
			}
		}

		virtual void update()
		{
			if (updateFlag)
			{
				for (auto& func : updateFuncs) {
					func(shared_from_this());
				}
			}
		}

		engineObject(const SDL_FRect& hull, SDL_Texture* tex, double rot = 0,
			bool centered = true, SDL_FlipMode flip = SDL_FLIP_NONE, float scale = 1.0,
			int depth = 0)
			: hull(hull), tex(tex), rot(rot),
			centered(centered), flip(flip), scale(scale),
			depth(depth),
			drawDefault(true), drawFlag(true), updateFlag(true), remove(false) {}

		virtual ~engineObject() {}
	};


	struct buttonObject : public Engine::engineObject
	{
		std::function<void()> onClick;

		void update()
		{
			if (mouseStates[0] == 1 && mouseInBounds())
				onClick();
			for (auto& func : updateFuncs) {
				func(shared_from_this());
			}
		}

		buttonObject(const SDL_FRect& hull, SDL_Texture* tex, double rot = 0,
			bool centered = true, SDL_FlipMode flip = SDL_FLIP_NONE, float scale = 1.0,
			SDL_FPoint vel = { 0, 0 }, double spin = 0, int depth = 0)
			: engineObject(hull, tex, rot,
				centered, flip, scale,
				depth),
			onClick(onClick) {}
	};

	static std::shared_ptr<Engine::engineObject> addObject(std::shared_ptr<Engine::engineObject> obj);

};
#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_image.h>
#include <SDL3/SDL_ttf.h>
#include <vector>
#include <functional>
#include <memory>
#include <string>
#include <time.h>


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
	static float zoom;
	static SDL_FPoint camPos;
	static SDL_FRect screenBounds;
	static SDL_FPoint screenOffSet;
	static double screenRotation;
	static SDL_FPoint mousePos;
	static SDL_FPoint rawMousePos;
	static SDL_FPoint mousePosDif;
	static std::vector<int> mouseStates;
	static std::vector<int> keyStates;
	static std::vector<int> wheelStates;
	static float deltaSeconds;

	static void clearTarget(SDL_Color color = { 0,0,0,255 });
	static SDL_Texture* setRenderTarget(SDL_Texture* tex);
	static SDL_Point setResolution(SDL_Point res);
	static bool toggleVsync();
	static TTF_Font* loadFont(const char* path, int size);
	static SDL_Texture* loadText(const char* text, TTF_Font* font, SDL_Color color);
	static SDL_Texture* loadTex(const char* file, bool unique = false);
	static SDL_Texture* loadTargetTex(SDL_Point size);
	static void drawLine(SDL_FPoint start, SDL_FPoint end, SDL_Color color = { 255, 255, 255, 255 });
	static void drawRect(SDL_FRect rect, SDL_Color color = { 255, 255, 255, 255 }, bool fill = false, bool centered = false);
	static void drawTex(SDL_Texture* tex, SDL_FRect rect, double rot = 0.0, bool center = true, SDL_FlipMode flip = SDL_FLIP_NONE, float scale = 1.0, SDL_FRect* chunk = NULL);
	static void removeAllObjects();
	static void draw();
	static void controller();
	static bool initEngine(const char* title = "CadEngine", SDL_WindowFlags winFlags = NULL);

	struct engineObject : public std::enable_shared_from_this<engineObject> {
		std::vector<SDL_Texture*> tex;
		int texIndex;
		bool centered;
		bool fixed;
		SDL_FlipMode flip;
		float scale;
		SDL_FRect hull;
		double rot;
		int depth;
		bool drawDefault;
		bool drawFlag;
		bool updateFlag;
		bool remove;
		clock_t timeCreated;
		std::vector<std::function<void(std::shared_ptr<Engine::engineObject> obj)>> drawFuncs;
		std::vector<std::function<void(std::shared_ptr<Engine::engineObject> obj)>> updateFuncs;


		virtual void drawHull()
		{
			SDL_Color color = { 255, 255, 255, 255 };
			if (fixed)
				color = { 255, 0, 0, 255 };
			if (centered)
				color = { 0, 255, 0, 255 };
			if (fixed && centered)
				color = { 0, 0, 255, 255 };
			if (mouseInBounds())
				color.a = 255 / 2;
			SDL_FRect modHull = hull;
			if (!fixed)
			{
				modHull.x -= camPos.x;
				modHull.y -= camPos.y;
				modHull.x *= zoom;
				modHull.y *= zoom;
				modHull.x += baseRes.x / 2;
				modHull.y += baseRes.y / 2;

				modHull.w *= scale * zoom;
				modHull.h *= scale * zoom;
			}
			if (centered)
			{
				modHull.x -= modHull.w / 2;
				modHull.y -= modHull.h / 2;
			}
			drawRect(modHull, color);
			Engine::drawLine({ modHull.x, modHull.y }, { modHull.x + modHull.w - 1, modHull.y + modHull.h - 1 }, color);
			Engine::drawLine({ modHull.x, modHull.y + modHull.h - 1 }, { modHull.x + modHull.w - 1, modHull.y }, color);
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

		bool inScreen()
		{
			if (fixed)
				return true;
			SDL_FRect bounds = getBounds();
			if (bounds.x <= Engine::screenBounds.w && bounds.w >= Engine::screenBounds.x &&
				bounds.y <= Engine::screenBounds.h && bounds.h >= Engine::screenBounds.y)
			{
				return true;
			}
			return false;
		}

		virtual bool mouseInBounds()
		{
			SDL_FRect bounds = getBounds();
			SDL_FPoint checkPoint = mousePos;
			if (fixed)
				checkPoint = rawMousePos;
			if (checkPoint.x >= bounds.x && checkPoint.x <= bounds.w && checkPoint.y >= bounds.y && checkPoint.y <= bounds.h)
			{
				return true;
			}
			return false;
		}

		void resetSize()
		{
			float w, h;
			SDL_GetTextureSize(tex.front(), &w, &h);
			hull.w = w;
			hull.h = h;
			scale = 1;
		}

		virtual void draw()
		{
			if (drawFlag && inScreen())
			{
				if (drawDefault)
				{
					SDL_FRect modHull = hull;
					if (!fixed)
					{
						modHull.x -= camPos.x;
						modHull.y -= camPos.y;
						modHull.x *= zoom;
						modHull.y *= zoom;
						modHull.x += baseRes.x / 2;
						modHull.y += baseRes.y / 2;

						modHull.w *= zoom;
						modHull.h *= zoom;
					}
					drawTex(tex[texIndex], modHull, rot, centered, flip, scale);
				}
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

		engineObject(const SDL_FRect& hull = { 0,0,10,10 }, SDL_Texture* tex = NULL, double rot = 0,
			bool centered = true, bool fixed = false, SDL_FlipMode flip = SDL_FLIP_NONE, float scale = 1.0,
			int depth = 0)
			: hull(hull), tex{ tex }, rot(rot), centered(centered), fixed(fixed),
			flip(flip), scale(scale), depth(depth),
			drawDefault(true), drawFlag(true), updateFlag(true), remove(false), texIndex(0) {
			timeCreated = clock();
		}

		virtual ~engineObject() {}
	};


	struct buttonObject : public Engine::engineObject
	{
		std::function<void()> onClick;

		void update()
		{
			if (updateFlag && mouseStates[0] == 1 && mouseInBounds())
				onClick();
			for (auto& func : updateFuncs) {
				func(shared_from_this());
			}
		}

		buttonObject(const SDL_FRect& hull, SDL_Texture* tex, double rot = 0,
			bool centered = true, bool fixed = true, SDL_FlipMode flip = SDL_FLIP_NONE, float scale = 1.0,
			SDL_FPoint vel = { 0, 0 }, double spin = 0, int depth = 0)
			: engineObject(hull, tex, rot,
				centered, fixed, flip, scale,
				depth),
			onClick(onClick) {}
	};

	static std::shared_ptr<Engine::engineObject> addObject(std::shared_ptr<Engine::engineObject> obj);

};
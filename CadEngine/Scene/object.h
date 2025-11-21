#pragma once
#include <SDL3/SDL.h>
#include <vector>
#include <functional>
#include <memory>
#include <time.h>


static class Object {
public:
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
		std::vector<std::function<void(std::shared_ptr<engineObject> obj)>> drawFuncs;
		std::vector<std::function<void(std::shared_ptr<engineObject> obj)>> updateFuncs;
		std::vector<std::function<void(std::shared_ptr<engineObject> obj)>> startupFuncs;


		virtual void drawHull();
		SDL_FRect getBounds();
		bool inScreen();
		virtual bool mouseInBounds();
		void resetSize();
		virtual void draw();
		virtual void update();

		engineObject(
			const SDL_FRect& hull = { 0,0,10,10 },
			SDL_Texture* tex = NULL,
			double rot = 0,
			bool centered = true,
			bool fixed = false,
			SDL_FlipMode flip = SDL_FLIP_NONE,
			float scale = 1.0f,
			int depth = 0);
		virtual ~engineObject();
	};

	struct buttonObject : public engineObject
	{
		std::function<void()> onClick;
		std::function<void(std::shared_ptr<engineObject> obj)> onHover;
		std::function<void(std::shared_ptr<engineObject> obj)> offHover;

		void update();
		buttonObject(
			const SDL_FRect& hull = { 0, 0, 0, 0 },
			SDL_Texture* tex = NULL,
			double rot = 0,
			bool centered = true,
			bool fixed = true,
			SDL_FlipMode flip = SDL_FLIP_NONE,
			float scale = 1.0,
			SDL_FPoint vel = { 0, 0 },
			double spin = 0,
			int depth = 0);
	};

};
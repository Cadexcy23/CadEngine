#pragma once
#include <SDL3/SDL.h>
#include <vector>
#include <functional>
#include <memory>
#include <time.h>
#include "../Graphics/renderer.h"
#include "../Graphics/texture.h"


static class Object {
public:
	class engineObjectBase : public std::enable_shared_from_this<engineObjectBase> {
	public:
		std::vector<SDL_Texture*> textures;
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
		/*std::vector<std::function<void(std::shared_ptr<Derived> obj)>> drawFuncs;
		std::vector<std::function<void(std::shared_ptr<Derived> obj)>> updateFuncs;
		std::vector<std::function<void(std::shared_ptr<Derived> obj)>> spawnFuncs;
		std::vector<std::function<void(std::shared_ptr<Derived> obj)>> despawnFuncs;*/


		virtual void drawHull();
		SDL_FRect getBounds();
		bool inScreen();
		virtual bool mouseInBounds();
		void resetSize();
		virtual void update();
		virtual void draw();


		engineObjectBase(
			const SDL_FRect& hull = { 0,0,10,10 },
			std::vector<SDL_Texture*> textures = {},
			double rot = 0,
			bool centered = true,
			bool fixed = false,
			SDL_FlipMode flip = SDL_FLIP_NONE,
			float scale = 1.0f,
			int depth = 0);
		virtual ~engineObjectBase();
	};

	// Template for typed functionality
	template<typename Derived>
	class engineObject : public engineObjectBase {
	protected:
		std::vector<std::function<void(std::shared_ptr<Derived>)>> drawFuncs;
		std::vector<std::function<void(std::shared_ptr<Derived>)>> updateFuncs;
		std::vector<std::function<void(std::shared_ptr<Derived>)>> spawnFuncs;
		std::vector<std::function<void(std::shared_ptr<Derived>)>> despawnFuncs;

	public:
		// Typed function management
		void addUpdateFunc(std::function<void(std::shared_ptr<Derived>)> func) {
			updateFuncs.push_back(func);
		}

		void addSpawnFunc(std::function<void(std::shared_ptr<Derived>)> func) {
			spawnFuncs.push_back(func);
		}

		// Get typed shared_ptr
		std::shared_ptr<Derived> sharedFromDerived() {
			return std::static_pointer_cast<Derived>(this->shared_from_this());
		}

		// Implementation of virtual methods
		void update() override {
			auto self = sharedFromDerived();

			// Process spawn functions
			for (auto& func : spawnFuncs) {
				func(self);
			}
			spawnFuncs.clear();

			// Process update functions
			if (updateFlag) {
				for (auto& func : updateFuncs) {
					func(self);
				}
			}
		}

		void draw() override {
			if (drawFlag && inScreen())
			{
				auto self = sharedFromDerived();
				if (drawDefault)
				{
					SDL_FRect modHull = hull;
					if (!fixed)
					{
						modHull.x -= Renderer::camPos.x;
						modHull.y -= Renderer::camPos.y;
						modHull.x *= Renderer::zoom;
						modHull.y *= Renderer::zoom;
						modHull.x += Renderer::baseRes.x / 2;
						modHull.y += Renderer::baseRes.y / 2;

						modHull.w *= Renderer::zoom;
						modHull.h *= Renderer::zoom;
					}
					Texture::drawTex(textures[texIndex], modHull, rot, centered, flip, scale);
				}
				if (false) //FIX: need debug hull draw flag in renderer
					drawHull();
				for (auto& func : drawFuncs) {
					func(self);
				}
			}
		}

		engineObject(
			const SDL_FRect& hull = { 0,0,10,10 },
			std::vector<SDL_Texture*> textures = {},
			double rot = 0,
			bool centered = true,
			bool fixed = false,
			SDL_FlipMode flip = SDL_FLIP_NONE,
			float scale = 1.0f,
			int depth = 0) 
			: engineObjectBase(hull, textures, rot, 
				centered, fixed, flip, scale, depth) {
		}
	};

	struct buttonObject : public engineObject<buttonObject>
	{
		std::function<void()> onClick;
		std::function<void(std::shared_ptr<engineObjectBase> obj)> onHover;
		std::function<void(std::shared_ptr<engineObjectBase> obj)> offHover;


		void update();


		buttonObject(
			const SDL_FRect& hull = { 0, 0, 0, 0 },
			std::vector<SDL_Texture*> textures = {},
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
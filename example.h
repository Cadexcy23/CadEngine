#pragma once
#include "engine.h"

static class Example {
public:

	struct velObject : public Engine::engineObject
	{
		SDL_FPoint vel;
		double spin;

		void draw() {
			if (drawFlag && inScreen())
			{
				if (drawDefault)
				{
					SDL_FRect modHull = hull;
					if (!fixed)
					{
						modHull.x -= Engine::camPos.x;
						modHull.y -= Engine::camPos.y;
						modHull.x *= Engine::zoom;
						modHull.y *= Engine::zoom;
						modHull.x += Engine::baseRes.x / 2;
						modHull.y += Engine::baseRes.y / 2;

						modHull.w *= Engine::zoom;
						modHull.h *= Engine::zoom;
					}
					Engine::drawTex(tex[texIndex], modHull, rot, centered, flip, scale);
				}
				if (Engine::debugLevel)
					drawHull();
				for (auto& func : drawFuncs) {
					func(shared_from_this());
				}
			}
		}

		velObject(const SDL_FRect hull, SDL_Texture* tex, double rot = 0,
			bool centered = true, bool fixed = false, SDL_FlipMode flip = SDL_FLIP_NONE, float scale = 1.0,
			SDL_FPoint vel = { 0, 0 }, double spin = 0, int depth = 0)
			: engineObject(hull, tex, rot,
				centered, fixed, flip, scale,
				depth),
			vel(vel), spin(spin) {}
	};

};

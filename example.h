#pragma once
#include "engine.h"

static class Example {
public:

	struct velObject : public Engine::engineObject
	{
		SDL_FPoint vel;
		double spin;

		velObject(const SDL_FRect& hull, SDL_Texture* tex, double rot = 0,
			bool centered = true, bool fixed = false, SDL_FlipMode flip = SDL_FLIP_NONE, float scale = 1.0,
			SDL_FPoint vel = { 0, 0 }, double spin = 0, int depth = 0)
			: engineObject(hull, tex, rot,
				centered, fixed, flip, scale,
				depth),
			vel(vel), spin(spin) {}
	};

};

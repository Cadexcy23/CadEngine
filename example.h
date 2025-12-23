#pragma once
#include "CadEngine/Core/engine.h"

static class Example {
public:

	struct velObject : public Object::engineObject<velObject>
	{
		SDL_FPoint vel;
		double spin;

		velObject(const SDL_FRect hull = {0, 0, 10, 10}, std::vector<SDL_Texture*> textures = {}, double rot = 0,
			bool centered = true, bool fixed = false, SDL_FlipMode flip = SDL_FLIP_NONE, float scale = 1.0,
			SDL_FPoint vel = { 0, 0 }, double spin = 0, int depth = 0)
			: engineObject(hull, textures, rot,
				centered, fixed, flip, scale,
				depth),
			vel(vel), spin(spin) {}
	};

};

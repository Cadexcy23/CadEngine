#include "object.h"
#include "../Core/engine.h"

//EngineObject
void Object::engineObjectBase::drawHull()
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
		modHull.x -= Renderer::camPos.x;
		modHull.y -= Renderer::camPos.y;
		modHull.x *= Renderer::zoom;
		modHull.y *= Renderer::zoom;
		modHull.x += Renderer::baseRes.x / 2;
		modHull.y += Renderer::baseRes.y / 2;

		modHull.w *= scale * Renderer::zoom;
		modHull.h *= scale * Renderer::zoom;
	}
	if (centered)
	{
		modHull.x -= modHull.w / 2;
		modHull.y -= modHull.h / 2;
	}
	Renderer::drawRect(modHull, color);
	Renderer::drawLine({ modHull.x, modHull.y }, { modHull.x + modHull.w - 1, modHull.y + modHull.h - 1 }, color);
	Renderer::drawLine({ modHull.x, modHull.y + modHull.h - 1 }, { modHull.x + modHull.w - 1, modHull.y }, color);
}

SDL_FRect Object::engineObjectBase::getBounds()
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

bool Object::engineObjectBase::inScreen()
{
	if (fixed)
		return true;
	SDL_FRect bounds = getBounds();
	if (bounds.x <= Renderer::screenBounds.w && bounds.w >= Renderer::screenBounds.x &&
		bounds.y <= Renderer::screenBounds.h && bounds.h >= Renderer::screenBounds.y)
	{
		return true;
	}
	return false;
}

bool Object::engineObjectBase::mouseInBounds()
{
	SDL_FRect bounds = getBounds();
	SDL_FPoint checkPoint = Input::mousePos;
	if (fixed)
		checkPoint = Input::rawMousePos;
	if (checkPoint.x >= bounds.x && checkPoint.x <= bounds.w && checkPoint.y >= bounds.y && checkPoint.y <= bounds.h)
	{
		return true;
	}
	return false;
}

void Object::engineObjectBase::resetSize()
{
	float w, h;
	SDL_GetTextureSize(textures.front(), &w, &h);
	hull.w = w;
	hull.h = h;
	scale = 1;
}

Object::engineObjectBase::engineObjectBase(
	const SDL_FRect& hull,
	std::vector<SDL_Texture*> textures,
	double rot,
	bool centered,
	bool fixed,
	SDL_FlipMode flip,
	float scale,
	int depth
) : hull(hull), textures(textures), rot(rot), centered(centered), fixed(fixed),
flip(flip), scale(scale), depth(depth),
drawDefault(true), drawFlag(true), updateFlag(true), remove(false), texIndex(0)
{
	timeCreated = clock();
}

Object::engineObjectBase::~engineObjectBase()
{
	Logger::log(Logger::LogCategory::Engine, Logger::LogLevel::Trace, "EngineObject destroyed");
}

//ButtonObject
void Object::buttonObject::update()
{
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

		// Extra button functionality
		if (mouseInBounds())
		{
			if (onHover)
				onHover(self);
			if (onClick && Input::mouseStates[0] == 1)
				onClick();
		}
		else if (offHover)
			offHover(self);
	}
}

Object::buttonObject::buttonObject(
	const SDL_FRect& hull,
	std::vector<SDL_Texture*> textures,
	double rot,
	bool centered,
	bool fixed,
	SDL_FlipMode flip,
	float scale,
	SDL_FPoint vel,
	double spin,
	int depth)
	: engineObject(hull, textures, rot,
		centered, fixed, flip, scale,
		depth),
	onClick(onClick) {
}
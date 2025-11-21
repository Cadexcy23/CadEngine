#include "object.h"
#include "../Core/engine.h"


void Object::engineObject::drawHull()
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

SDL_FRect Object::engineObject::getBounds()
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

bool Object::engineObject::inScreen()
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

bool Object::engineObject::mouseInBounds()
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

void Object::engineObject::resetSize()
{
	float w, h;
	SDL_GetTextureSize(tex.front(), &w, &h);
	hull.w = w;
	hull.h = h;
	scale = 1;
}

void Object::engineObject::draw()
{
	if (drawFlag && inScreen())
	{
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
			Texture::drawTex(tex[texIndex], modHull, rot, centered, flip, scale);
		}
		if (Engine::debugLevel)
			drawHull();
		for (auto& func : drawFuncs) {
			func(shared_from_this());
		}
	}
}

void Object::engineObject::update()
{
	for (auto& func : startupFuncs) {
		func(shared_from_this());
	}
	startupFuncs.clear();

	if (updateFlag)
	{
		for (auto& func : updateFuncs) {
			func(shared_from_this());
		}
	}
}

Object::engineObject::engineObject(
	const SDL_FRect& hull,
	SDL_Texture* texPtr,
	double rot,
	bool centered,
	bool fixed,
	SDL_FlipMode flip,
	float scale,
	int depth
) : hull(hull), rot(rot), centered(centered), fixed(fixed),
flip(flip), scale(scale), depth(depth),
drawDefault(true), drawFlag(true), updateFlag(true), remove(false), texIndex(0)
{
	timeCreated = clock();
	if (texPtr)
		tex.push_back(texPtr);
}

Object::engineObject::~engineObject()
{
}

void Object::buttonObject::update()
{
	if (updateFlag && mouseInBounds())
	{
		if (onHover)
			onHover(shared_from_this());
		if (onClick && Input::mouseStates[0] == 1)
			onClick();
	}
	else if (offHover)
		offHover(shared_from_this());
	for (auto& func : updateFuncs) {
		func(shared_from_this());
	}
}

Object::buttonObject::buttonObject(
	const SDL_FRect& hull,
	SDL_Texture* tex,
	double rot,
	bool centered,
	bool fixed,
	SDL_FlipMode flip,
	float scale,
	SDL_FPoint vel,
	double spin,
	int depth)
	: engineObject(hull, tex, rot,
		centered, fixed, flip, scale,
		depth),
	onClick(onClick) {
}
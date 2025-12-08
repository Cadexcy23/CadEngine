#include "texture.h"
#include "renderer.h"
#include "../Core/logger.h"

std::vector<std::pair<std::string, SDL_Texture*>> Texture::activeTextures;


void Texture::clearTarget(SDL_Color color)
{
	Uint8 r, g, b, a;
	SDL_GetRenderDrawColor(Renderer::renderer, &r, &g, &b, &a);
	SDL_SetRenderDrawColor(Renderer::renderer, color.r, color.g, color.b, color.a);
	SDL_RenderClear(Renderer::renderer);
	SDL_SetRenderDrawColor(Renderer::renderer, r, g, b, a);
}

SDL_Texture* Texture::setRenderTarget(SDL_Texture* textures)
{
	SDL_Texture* lastTex = SDL_GetRenderTarget(Renderer::renderer);
	SDL_SetRenderTarget(Renderer::renderer, textures);
	return lastTex;
}

SDL_Texture* Texture::clearTextureChunk(SDL_Texture* textures, SDL_FRect chunk)
{
	//if default value, do full clear
	if (!chunk.w)
	{
		//set render target
		SDL_Texture* lastTex = setRenderTarget(textures);

		//clear tex
		clearTarget({ 128, 128, 128, 0 });

		//return our render target to previous
		setRenderTarget(lastTex);

		return textures;
	}

	//create new texture of the same size
	SDL_Texture* newTex = loadTargetTex({ textures->w, textures->h });

	//set render target
	SDL_Texture* lastTex = setRenderTarget(newTex);

	//clear tex
	clearTarget({ 128, 128, 128, 0 });

	//determine the 4 rects around the chunk we need to copy
	SDL_FRect rects[4];
	rects[0] = { 0, 0, chunk.x + chunk.w, chunk.y };
	rects[1] = { chunk.x + chunk.w, 0, float(textures->w) - chunk.x - chunk.w, chunk.y + chunk.h };
	rects[2] = { chunk.x, chunk.y + chunk.h, float(textures->w) - chunk.x, float(textures->h) - chunk.y - chunk.h };
	rects[3] = { 0, chunk.y, chunk.x, float(textures->h) - chunk.y };

	//draw these new rects onto the new texture from the old one
	for (auto r : rects)
		drawTex(textures, r, 0, false, SDL_FLIP_NONE, 1, &r);

	//return our render target to previous
	setRenderTarget(lastTex);

	return newTex;
}

void Texture::cleanActiveTextures()
{
	for (auto it = Texture::activeTextures.begin(); it != Texture::activeTextures.end();)
	{
		if (it->second->format == SDL_PIXELFORMAT_ARGB8888 ||
			it->second->format == SDL_PIXELFORMAT_ABGR8888)
			++it;
		else
		{
			//remove texture from memory
			SDL_DestroyTexture(it->second);
			//remove texture from list
			//Engine::log(("Destroyed texture: " + it->first).c_str());
			it = Texture::activeTextures.erase(it);
		}
	}
}

SDL_Texture* Texture::loadTex(const char* path, bool unique)
{
	//clean out lost textures
	cleanActiveTextures();

	//Check if we already have this texture loaded
	if (!unique)//Skip if we want a unique one
	{
		for (const auto& textures : Texture::activeTextures)
		{
			//If we already have the texture loaded just return it
			if (textures.first == path)
			{
				//Engine::log(("Reusing loaded texture: " + std::string(path)).c_str());
				return textures.second;
			}
		}
	}

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path);
	if (loadedSurface == NULL)
	{
		Logger::log(Logger::LogCategory::Graphics, Logger::LogLevel::Error, "Unable to load image %s! SDL_image Error: %s", path, SDL_GetError());
		return NULL;
	}
	//Create texture from surface pixels
	SDL_Texture* newTexture = SDL_CreateTextureFromSurface(Renderer::renderer, loadedSurface);
	if (newTexture == NULL)
	{
		Logger::log(Logger::LogCategory::Graphics, Logger::LogLevel::Error, "Unable to create texture from %s! SDL Error: %s", path, SDL_GetError());
		return NULL;
	}
	//Get rid of old loaded surface
	SDL_DestroySurface(loadedSurface);

	//Set scale mode for texture
	SDL_SetTextureScaleMode(newTexture, Renderer::scaleMode);

	//Add tex to our list of loaded textures
	if (!unique)
		Texture::activeTextures.push_back({ path, newTexture });

	/*if (!unique)
		Engine::log(("New texture created: " + std::string(path)).c_str());
	else
		Engine::log(("Unique texture loaded: " + std::string(path)).c_str());*/

	return newTexture;
}

SDL_Texture* Texture::loadTargetTex(SDL_Point size)
{
	//Load texture with the target flag
	SDL_Texture* newTexture = SDL_CreateTexture(Renderer::renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, size.x, size.y);

	//Set scale mode for texture
	SDL_SetTextureScaleMode(newTexture, Renderer::scaleMode);

	return newTexture;
}

void Texture::drawTex(SDL_Texture* textures, SDL_FRect rect, double rot, bool center, SDL_FlipMode flip, float scale, SDL_FRect* chunk)
{
	SDL_FRect newRect = rect;
	if (newRect.w == 0 || newRect.h == 0)
	{
		newRect.w = float(textures->w);
		newRect.h = float(textures->h);
	}
	newRect.w *= scale;
	newRect.h *= scale;
	switch (center)
	{
	case true:
		newRect.x -= newRect.w / 2;
		newRect.y -= newRect.h / 2;
		SDL_RenderTextureRotated(Renderer::renderer, textures, chunk, &newRect, rot, NULL, flip);
		break;
	case false:
		SDL_RenderTextureRotated(Renderer::renderer, textures, chunk, &newRect, rot, NULL, flip);
		break;
	}
}
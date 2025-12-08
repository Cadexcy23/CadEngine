#include "text.h"
#include "texture.h"
#include "renderer.h"
#include "../Core/logger.h"


std::vector<std::pair<std::string, TTF_Font*>> Text::activeFonts;


TTF_Font* Text::loadFont(const char* path, int size)
{
	//Check if we already have this font loaded
	for (const auto& font : activeFonts)
	{
		//If we already have the texture loaded just return it
		if (font.first == path)
			return font.second;
	}
	TTF_Font* font = TTF_OpenFont(path, size);
	if (font == NULL)
	{
		Logger::log(Logger::LogCategory::Graphics, Logger::LogLevel::Error, "Failed to load font! SDL_ttf Error: %s", SDL_GetError());
		return NULL;
	}

	//Add font to our list of loaded textures
	activeFonts.push_back({ path, font });

	return font;
}

SDL_Texture* Text::loadText(const char* text, TTF_Font* font, SDL_Color color)
{
	//Check if we already have this text loaded
	for (const auto& textures : Texture::activeTextures)
	{
		//If we already have the text loaded just return it
		if (textures.first == std::string(text) + std::to_string(int(font)) + std::to_string(color.r) + std::to_string(color.g) + std::to_string(color.b) + std::to_string(color.a))
			return textures.second;
	}

	//Load image at specified path
	SDL_Surface* loadedSurface = TTF_RenderText_Blended(font, text, NULL, color);
	if (loadedSurface == NULL)
	{
		Logger::log(Logger::LogCategory::Graphics, Logger::LogLevel::Error, "Unable to load text %s! SDL_image Error: %s", text, SDL_GetError());
		return NULL;
	}
	//Create texture from surface pixels
	SDL_Texture* newTexture = SDL_CreateTextureFromSurface(Renderer::renderer, loadedSurface);
	if (newTexture == NULL)
	{
		Logger::log(Logger::LogCategory::Graphics, Logger::LogLevel::Error, "Unable to create texture from %s! SDL Error: %s", text, SDL_GetError());
		return NULL;
	}
	//Get rid of old loaded surface
	SDL_DestroySurface(loadedSurface);

	//Set scale mode for texture
	SDL_SetTextureScaleMode(newTexture, Renderer::scaleMode);

	//Add tex to our list of loaded textures
	Texture::activeTextures.push_back({ std::string(text) + std::to_string(int(font)) + std::to_string(color.r) + std::to_string(color.g) + std::to_string(color.b) + std::to_string(color.a), newTexture });

	return newTexture;
}

bool Text::initFont()
{
	//Initialize SDL_ttf
	if (TTF_Init() == -1)
	{
		Logger::log(Logger::LogCategory::Graphics, Logger::LogLevel::Error, "SDL_ttf could not initialize! SDL_ttf Error: %s", SDL_GetError());
		return false;
	}
}

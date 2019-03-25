#include "Sprite.h"
#include <iostream>

Sprite::Sprite(int x, int y, int width, int height)
{
	m_bounds = { x, y, width, height };
}

Sprite::Sprite(const SDL_Rect rect) :
	m_bounds(rect)
{
}

Sprite::~Sprite()
{
	SDL_DestroyTexture(m_texture);
}

SDL_Texture * Sprite::loadTexture(std::string path, SDL_Renderer * renderer)
{
	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}
	m_texture = newTexture;
	/*if (!m_bounds.w && !m_bounds.h) {
	SDL_QueryTexture(m_texture, NULL, NULL, &m_bounds.w, &m_bounds.h);
	}*/
	return newTexture;
}

void Sprite::render(SDL_Renderer * renderer, int angle)
{
	SDL_RenderCopyEx(renderer, m_texture, NULL, &m_bounds, angle, NULL, SDL_RendererFlip::SDL_FLIP_NONE);
}

void Sprite::setBounds(int w, int h)
{
	m_bounds.w = w;
	m_bounds.h = h;
}

void Sprite::setPosition(int x, int y)
{
	m_bounds.x = x;
	m_bounds.y = y;
}

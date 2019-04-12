#include "Sprite.h"
#include <iostream>

/// <summary>
/// Default sprite constructor
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="width"></param>
/// <param name="height"></param>
Sprite::Sprite(int x, int y, int width, int height)
{
	m_bounds = { x, y, width, height };
}

/// <summary>
/// Construct a sprite using an sdl rect
/// </summary>
/// <param name="rect">SDL rect representing bounds</param>
Sprite::Sprite(const SDL_Rect rect) :
	m_bounds(rect)
{
}

/// <summary>
/// Sprite deconstructing
/// </summary>
Sprite::~Sprite()
{
	SDL_DestroyTexture(m_texture);
}

/// <summary>
/// Load the sprite texture and map it to the sprite
/// </summary>
/// <param name="path">Filepath to sprite to load</param>
/// <param name="renderer">The sdl Renderer to use for texture construction</param>
/// <returns>Pointer to an sdl texture</returns>
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

/// <summary>
/// Render the sprite
/// </summary>
/// <param name="renderer">SDL renderer to render to</param>
/// <param name="angle">Angle to render the sprite at</param>
void Sprite::render(SDL_Renderer * renderer, int angle)
{
	SDL_RenderCopyEx(renderer, m_texture, NULL, &m_bounds, angle, NULL, SDL_RendererFlip::SDL_FLIP_NONE);
}

/// <summary>
/// Set the bounds of the sprite
/// </summary>
/// <param name="w">Sprite bounds width</param>
/// <param name="h">Sprite bounds height</param>
void Sprite::setBounds(int w, int h)
{
	m_bounds.w = w;
	m_bounds.h = h;
}

/// <summary>
/// Set the position of the sprite
/// </summary>
/// <param name="x">X position</param>
/// <param name="y">Y position</param>
void Sprite::setPosition(int x, int y)
{
	m_bounds.x = x;
	m_bounds.y = y;
}

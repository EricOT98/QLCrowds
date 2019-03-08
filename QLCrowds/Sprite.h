#ifndef SPRITE_H
#define SPRITE_H

#include <SDL.h>
#include <SDL_image.h>
#include <string>

class Sprite {
public:
	Sprite(int x = 0, int y = 0, int width = 0, int height = 0);
	Sprite(const SDL_Rect rect);
	~Sprite();
	virtual SDL_Texture* loadTexture(std::string path, SDL_Renderer * renderer);
	virtual void render(SDL_Renderer* renderer);
	void setBounds(int w, int h);
	void setPosition(int x, int y);
protected:
	SDL_Texture * m_texture;
	SDL_Rect m_bounds;
};

#endif //!SPRITE_H
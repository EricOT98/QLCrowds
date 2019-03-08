#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <SDL.h>

#include "Environment.h"
#include "Agent.h"

class Game {
public:
	Game();
	~Game();

	void update(float deltaTime);
	void render();
	void run();
	void processEvents();
private:
	SDL_Window * m_window;
	SDL_Renderer* m_renderer;
	const int m_windowWidth = 1280;
	const int m_windowHeight = 720;

	//Loop Content
	bool m_quit;
	SDL_Event m_event;
	Environment env;
	Agent agent;
};

#endif // !GAME_H
#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <SDL.h>

#include "Environment.h"
#include "Agent.h"

struct EpisodeVals {
	int action;
	std::pair<int, int> state;
	std::pair<int, int> nextState;
	float reward;
};

class Game {
public:
	Game();
	~Game();

	void update(float deltaTime);
	void render();
	void run();
	void processEvents();
	void saveEpisode();
private:
	SDL_Window * m_window;
	SDL_Renderer* m_renderer;
	const int m_windowWidth = 1280;
	const int m_windowHeight = 720;

	//Loop Content
	bool m_quit;
	SDL_Event m_event;
	Environment env;
	Agent * agent;
	std::vector<std::vector<EpisodeVals>> m_episodes;
	int currentEpisode = 0;
	int currentIteration;
	bool lerping = false;
	float lerpPercent = 0.03;
	float lerpMax = 1.f;
	float currentPercent = 0.0f;
};

#endif // !GAME_H
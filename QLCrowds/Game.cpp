#include "Game.h"
#include <iostream>
#include <SDL_image.h>

Game::Game() :
	agent(env)
{
	m_window = NULL;
	m_renderer = NULL;

	//if (SDL_Init(SDL_INIT_VIDEO) < 0) {
	//	std::cout << "SDL failed to initialize, SDL_Error: " << SDL_GetError() << std::endl;
	//}
	//else {
	m_window = SDL_CreateWindow("SDL: Command Pattern", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_windowWidth, m_windowHeight, SDL_WINDOW_SHOWN);
	if (!m_window) {
		std::cout << "SDL window failed to create, SDL_Error: " << SDL_GetError() << std::endl;
	}
	else {
		m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		if (!m_renderer) {
			std::cout << "Failed to create renderer, SDL_Erro: " << SDL_GetError();
		}
		SDL_RenderSetLogicalSize(m_renderer, m_windowWidth, m_windowHeight);
		SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
		//Fill the surface white
	}
	//}
	if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
	{
		printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
	}
	env.resizeGridTo(0, 0, 640, 360);
}

Game::~Game()
{
	SDL_DestroyRenderer(m_renderer);
	SDL_DestroyWindow(m_window);
	SDL_Quit();
}

void Game::update(float deltaTime)
{
	//std::cout << "				Update" << std::endl;
}

void Game::render()
{
	SDL_RenderClear(m_renderer);
	SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);

	//std::cout << "Pre: " << *r << "," << *g << "," << *b << "," << *a << std::endl;
	//Do stuff
	//m_test.render(m_renderer);
	env.render(*m_renderer);

	SDL_RenderPresent(m_renderer);
}

void Game::run()
{
	int numEpisodes = 500;
	for (int i = 0; i < numEpisodes; ++i) {
		std::cout << "Episode: " << i << std::endl;
		std::cout << "=================================================" << std::endl;
		int iter_episode = 0;
		float reward_episode = 0;
		auto state = env.reset();
		while (true) {
			//std::cout << "Iteration : " << index << std::endl;
			//std::cout << "%%%%%%%%%%%%%%" << std::endl;
			auto action = agent.getAction(env);
			auto state_vals = env.step(action);
			auto state_next = std::get<0>(state_vals);
			//std::cout << "S:" << state_next.first << "," << state_next.second << std::endl;
			auto reward = std::get<1>(state_vals);
			auto done = std::get<2>(state_vals);

			agent.train(std::make_tuple(state, action, state_next, reward, done));

			iter_episode++;
			reward_episode += reward;
			//std::cout << "R:" << reward_episode << std::endl;

			if (done) {
				break;
			}
			state = state_next;
		}
		agent.epsilon = std::fmax(agent.epsilon * agent.epsilonDecay, 0.01);

		std::cout << "Episode: " << i << " /" << numEpisodes << " Eps: " << agent.epsilon << " iter: " << iter_episode << " Rew: " << reward_episode << std::endl;
	}
	agent.displayGreedyPolicy();

	float timeSinceLastUpdate = 0.f;
	float timePerFrame = 1.f / 60.f; // 60 fps in ms
	float currentTime = SDL_GetTicks() / 1000.0f;
	while (!m_quit)
	{
		float frameStart = SDL_GetTicks() / 1000.0f;
		float frameTime = frameStart - currentTime;
		currentTime = frameStart;
		processEvents(); // as many as possible
		timeSinceLastUpdate += frameTime;
		while (timeSinceLastUpdate > timePerFrame)
		{
			timeSinceLastUpdate -= timePerFrame;
			processEvents(); // at least 60 fps
			update(timePerFrame); //60 fps
		}
		render(); // as many as possible
	}

}

void Game::processEvents()
{
	//std::cout << "PE" << std::endl;
	while (SDL_PollEvent(&m_event)) {
		switch (m_event.type) {
		case SDL_QUIT:
			m_quit = true;
			break;
		default:
			break;
		}
	}
}

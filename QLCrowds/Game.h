#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <SDL.h>
#include <thread>

#include "Environment.h"
#include "Agent.h"

#include "imgui/imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_sdl.h"

struct EpisodeVals {
	int action;
	std::pair<int, int> state;
	std::pair<int, int> nextState;
};

struct PlottableData {
	int episodeNumber;
	float rewardvalue;
};


struct AgentTrainingValues {
	AgentTrainingValues(Environment & env) : state(0, 0) {};
	int iter_episode = 0;
	float reward_episode = 0;
	std::pair<int, int> state;
};

class Game {
public:
	Game();
	~Game();
	std::thread agentSim(Agent * agent, std::vector<AgentTrainingValues> * agentVals, int currentAgent);

	void update(float deltaTime);
	void multiThreadedUpdate(float deltaTime);
	void render();
	void run();
	void processEvents();
	void saveEpisode();
	void runAlgorithm();
	void startSimulation();
	void resetSimulation();
	void resetAlgorithm();
	void renderUI();
private:
	SDL_Window * m_window;
	SDL_Renderer* m_renderer;
	const int m_windowWidth = 1280;
	const int m_windowHeight = 720;

	//Loop Content
	bool m_quit;
	SDL_Event m_event;
	Environment env;

	std::vector<Agent *> m_agents;
	int currentEpisode = 0;
	int currentIteration;
	bool lerping = false;
	float lerpPercent = 0.03;
	float lerpMax = 1.f;
	float currentPercent = 0.0f;
	bool m_algoStarted;
	bool m_algoFinished;
	bool m_simulationStarted;
	bool m_simulationFinished;
	int numEpisodes = 500;
	int maxIterations = 100;
	std::vector<std::vector<PlottableData>> plotPoints;
	const char* current_item = nullptr;
	const char* items[3] = { "Q Learning", "RBM" , "MultiRBM"};

	// Episode simulation Data
	std::vector<std::vector<std::vector<EpisodeVals>>> m_episodeData;
	std::vector<float> m_lerpPercentages;
	std::vector<bool> m_agentDone;
	std::vector<bool> m_agentLerping;
	std::vector<int> m_agentIterations;
	int m_numAgents = 1;

	void runRuleBased();
	void runQLearning();
	void runMARLQ();
	std::vector<std::thread> m_threads;
	bool m_multiThreaded = false;
};

#endif // !GAME_H
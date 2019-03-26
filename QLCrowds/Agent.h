#ifndef AGENT_H
#define AGENT_H

#include <iostream>
#include <vector>
#include <tuple>
#include <random>

#include "Environment.h"
#include "Sprite.h"
#include <SDL_render.h>

class Agent {
public:
	Agent(Environment & env, SDL_Renderer * renderer);
	~Agent();

	std::pair<int, int> stateDim;
	std::pair<int, int> actionDim;

	float epsilon = 1.f;			// Initial exploration prob
	float epsilonDecay = 0.99f;		//Epsilon decay after each episode
	float beta = 0.99f;				// Learning Rate
	float gamma = 0.99f;			//DIscount factor

	std::vector<std::vector<std::vector<float>>> Q; //Q Table for action state coupling
	bool m_done = false;

	// Backtracking controls
	std::pair<int, int> m_currentState;
	std::pair<int, int> m_previousState;
	bool m_backTracking = false;

	// General functions
	void reset();

	// Action functions
	int getAction(Environment & env);
	int getActionRBMBased(Environment & env);

	void train(std::tuple<std::pair<int,int>, int, std::pair<int, int>, float, bool> t);
	void trainRBM(std::tuple<std::pair<int, int>, int, std::pair<int, int>, float, bool> t);

	// Debug functions
	void displayGreedyPolicy();
	
	// Render functions
	void setOrientation(int action);
	void render(SDL_Renderer & renderer);
	Sprite m_sprite;
private:
	// Rendering
	int m_angle = 0;
};

#endif //!AGENT_H
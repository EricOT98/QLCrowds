#ifndef AGENT_H
#define AGENT_H

#include <iostream>
#include <vector>
#include <tuple>
#include <random>

#include "Environment.h"
#include "Sprite.h"
#include <SDL_render.h>
#include <tiny_dnn/tiny_dnn.h>

typedef std::pair<int, int> State;

class Agent {
public:
	struct AgentMemoryBatch {
		State state;
		State nextState;
		int action;
		float reward;
		bool done;
	};

	Agent(Environment & env, SDL_Renderer * renderer);
	~Agent();

	std::pair<int, int> stateDim;
	std::pair<int, int> actionDim;

	float epsilon = 1.f;			// Initial exploration prob
	float epsilonDecay = 0.9999f;		//Epsilon decay after each episode
	float beta = 0.99f;				// Learning Rate
	float gamma = 0.99f;			//DIscount factor

	std::vector<std::vector<std::vector<float>>> Q; //Q Table for action state coupling
	bool m_done = false;
	std::vector<AgentMemoryBatch> m_memory;

	// Backtracking controls
	std::pair<int, int> m_currentState;
	std::pair<int, int> m_previousState;
	bool m_backTracking = false;

	// General functions
	void reset();

	// Action functions
	int getAction(Environment & env);
	int getActionRBMBased(Environment & env);
	int getMultiAgentAction(Environment & env);
	int getMultiAgentActionRBM(Environment & env, int currentIter, const int maxIters);

	void train(std::tuple<std::pair<int,int>, int, std::pair<int, int>, float, bool> t);
	void trainRBM(std::tuple<std::pair<int, int>, int, std::pair<int, int>, float, bool> t);
	
	// NN function approximator work
	int trainStart = 100;
	int batchSize = 32;
	tiny_dnn::network<tiny_dnn::sequential> model;
	tiny_dnn::network<tiny_dnn::sequential> targetModel;
	tiny_dnn::network<tiny_dnn::sequential> buildModel();
	void updateTargetModel();
	void replayMemory(AgentMemoryBatch memory);
	void trainReplay();
	tiny_dnn::vec_t getBestActions(tiny_dnn::vec_t actions);

	// Debug functions
	void displayGreedyPolicy(Environment & env);
	
	// Render functions
	void setOrientation(int action);
	void render(SDL_Renderer & renderer);
	Sprite m_sprite;
	Environment & m_env;
private:
	// Rendering
	int m_angle = 0;
};

#endif //!AGENT_H
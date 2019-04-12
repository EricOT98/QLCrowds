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

	std::pair<int, int> m_stateDim;
	std::pair<int, int> m_actionDim;

	float m_epsilon = 1.f;			// Initial exploration prob
	float m_epsilonDecay = 0.9999f;	// Epsilon decay after each episode
	float m_beta = 0.99f;				// Learning Rate
	float m_gamma = 0.99f;			// Discount factor

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
	int getMultiAgentActionRBM(Environment & env, int currentIter, const int maxIters);

	// Learning function
	void train(std::tuple<std::pair<int,int>, int, std::pair<int, int>, float, bool> t);
	
	// NN function approximator work
	void updateTargetModel();
	void replayMemory(AgentMemoryBatch memory);
	void trainReplay();
	void resizeQTable();
	void resizeStates();
	void initModels();

	// Debug functions
	void displayGreedyPolicy(Environment & env);
	
	// Render functions
	void setOrientation(int action);
	void render(SDL_Renderer & renderer);
	

	// Setters
	void setPosition(float x, float y);
	void setSize(float w, float h);
private:
	// NN approximator work
	tiny_dnn::network<tiny_dnn::sequential> m_model;
	tiny_dnn::network<tiny_dnn::sequential> m_targetModel;
	int m_inputLayer;
	int m_outputLayer;
	int m_hiddenLayer;

	tiny_dnn::network<tiny_dnn::sequential> buildModel();

	int m_trainStart = 100;
	int m_batchSize = 32;
	int maxMemorySize = 1000;
	std::deque<AgentMemoryBatch> m_memory;

	Sprite m_sprite;
	Environment & m_env;
	// Rendering
	int m_angle = 0;
};

#endif //!AGENT_H
#ifndef AGENT_H
#define AGENT_H

#include <iostream>
#include <vector>
#include <tuple>

#include "Environment.h"
#include <random>

class Agent {
public:
	Agent(Environment & env);
	~Agent();

	std::pair<int, int> stateDim;
	std::pair<int, int> actionDim;

	float epsilon = 1.0f;			// Initial exploration prob
	float epsilonDecay = 0.99f;		//Epsilon decay after each episode
	float beta = 0.99f;				// Learning Rate
	float gamma = 0.99f;			//DIscount factor

	std::vector<std::vector<std::vector<float>>> Q;

	int getAction(Environment & env);
	void train(std::tuple<std::pair<int,int>, int, std::pair<int, int>, float, bool> t);
	void displayGreedyPolicy();
	std::default_random_engine generator;
};

#endif //!AGENT_H
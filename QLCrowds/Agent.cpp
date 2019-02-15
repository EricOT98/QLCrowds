#include "Agent.h"
#include <random>
#include <map>

Agent::Agent(Environment &env)
{
	stateDim = std::make_pair(env.ySize, env.xSize);
	actionDim = env.actionDim;
	Q.resize(stateDim.first);
	for (int row = 0; row < stateDim.first; ++row) {
		Q.at(row).resize(stateDim.second);
		for (int col = 0; col < stateDim.second; ++col) {
			Q.at(row).at(col).resize(actionDim.first);
		}
	}
	for (int row = 0; row < stateDim.second; ++row) {
		for (int col = 0; col < stateDim.first; ++col) {
			for (int action = 0; action < actionDim.first; ++action) {
				Q[row][col][action] = 0;
			}
		}
	}
}

Agent::~Agent()
{
}

float Agent::getAction(Environment & env)
{
	std::uniform_real_distribution<float> distribution(0.0, 1.0);
	//float randVal = distribution(generator);
	float randVal = 0.8;
	//std::cout << randVal << std::endl;
	if (randVal < epsilon) {
		//std::cout << "Explore" << std::endl;
		//Explore
		auto actions_allowed = env.allowedActions();
		int index = (std::rand() % (actions_allowed.size()));
		//std::cout << "Index:" << index << std::endl;
		return actions_allowed[index];
	}
	else {
		//std::cout << "Exploit" << std::endl;
		//Exploit
		std::pair<int, int> state = env.state;
		auto actions_allowed = env.allowedActions();

		std::vector<float> actionsToPickFrom;
		auto & availableActions = Q[state.first][state.second];

		std::map<int, float> actionsToEpsilon;

		for (auto & action : actions_allowed) {
			actionsToEpsilon.insert(std::make_pair(action,availableActions[action]));
		}
		
		std::vector<int> qS;
		auto maxElement = *(std::max_element(actionsToEpsilon.begin(), actionsToEpsilon.end()));
		std::vector<int> maximumActions;
		for (auto const & ae : actionsToEpsilon) {
			if (ae.second == maxElement.second) {
				maximumActions.push_back(ae.first);
			}
		}

		if (!maximumActions.empty()) {
			int index = std::rand() % maximumActions.size();
			//std::cout << maximumActions[index];
			return maximumActions[index];
		}
		else
			return 0.0f;
	}
}

void Agent::train(std::tuple<std::pair<int, int>, int, std::pair<int, int>, float, bool> t)
{
	//# -----------------------------
	//	# Update:
	//#
	//	# Q[s, a] <-Q[s, a] + beta * (R[s, a] + gamma * max(Q[s, :]) - Q[s, a])
	//#
	//		#  R[s, a] = reward for taking action a from state s
	//		#  beta = learning rate
	//		#  gamma = discount factor
	//# -----------------------------

	auto state = std::get<0>(t);
	int action = std::get<1>(t);
	auto state_next = std::get<2>(t);
	auto reward = std::get<3>(t);
	bool done = std::get<4>(t);

	//std::pair<int, int> sa(state.first,state.second);

	/*auto q = Q[state_next.first][state_next.second];
	float & actionToMod = Q[sa.first][sa.second][action];
	auto maxElement = std::max_element(q.begin(), q.end());
	actionToMod += beta * (reward + gamma * *(maxElement) - actionToMod);*/
	float & sa = Q[state.first][state.second][action];
	auto nextActions = Q[state_next.first][state_next.second];
	//self.Q[sa] += self.beta * (reward + self.gamma * np.max(self.Q[state_next]) - self.Q[sa])
	auto maxElement = *std::max_element(nextActions.begin(), nextActions.end());
	sa += beta * (reward + gamma * maxElement - sa);
}

void Agent::displayGreedyPolicy()
{
	std::vector<std::vector<float>> greedyPolicy;
	greedyPolicy.resize(stateDim.first);
	for (int row = 0; row < stateDim.first; ++row) {
		std::cout << "[";
		greedyPolicy.at(row).resize(stateDim.second);
		for (int col = 0; col < stateDim.second; ++col) {
			auto actions = Q[row][col];
			greedyPolicy[row][col] = *std::max_element(actions.begin(), actions.end());
			std::cout << greedyPolicy[row][col] << ",";
		}
		std::cout << "]," << std::endl;
	}
}

#include "Agent.h"
#include <random>
#include <map>

Agent::Agent(Environment &env, SDL_Renderer * renderer)
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
	reset();
	m_sprite.loadTexture("Assets/agent.png", renderer);
	m_sprite.setBounds(env.cellW, env.cellH);
}

Agent::~Agent()
{
}

int Agent::getAction(Environment & env)
{
	std::random_device rand_dev;
	std::mt19937 generator(rand_dev());
	std::uniform_real_distribution<double> distr(0, 1);
	double randVal = distr(generator);
	if (randVal < epsilon) {
		auto actions_allowed = env.allowedActions();
		std::uniform_int_distribution<int>  distr(0, actions_allowed.size() - 1);
		int index = distr(generator);
		return actions_allowed.at(index);
	}
	else {
		auto actions_allowed = env.allowedActions();
		auto & state = env.state;
		auto actionValues = Q[state.first][state.second];

		std::map<int, float> qs;
		for (int action : actions_allowed) {
			qs[action] = actionValues.at(action);
		}
		float maxVal = qs.begin()->second;
		for (auto & actionMapping : qs) {
			if (actionMapping.second > maxVal) {
				maxVal = actionMapping.second;
			}
		}
		std::vector<int> actions_greedy;
		for (auto & actionMapping : qs) {
			if (actionMapping.second == maxVal) {
				actions_greedy.push_back(actionMapping.first);
			}
		}
		std::uniform_int_distribution<int>  distr(0, actions_greedy.size() - 1);
		int index = distr(generator);
		return actions_greedy.at(index);
	}
}

/// <summary>
/// Train th eagent using the off policy Q learning method
/// </summary>
/// <param name="t"></param>
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

	float & sa = Q[state.first][state.second][action];
	auto nextActions = Q[state_next.first][state_next.second];

	auto maxElement = *std::max_element(nextActions.begin(), nextActions.end());
	Q[state.first][state.second][action] += beta * (reward + gamma * maxElement - sa);

}

int Agent::getActionRBMBased(Environment & env)
{
	auto allowedActions = env.allowedActions();
	auto & state = env.state;
	std::vector<int> possibleActions;
	std::pair<int, int> cellsfromGoal;
	cellsfromGoal.first = abs(stateDim.first - 1 - state.first);
	cellsfromGoal.second = abs(stateDim.second - 1 - state.second);
	for (auto & action : allowedActions) {
		std::pair<int, int> nextState;
		auto actionDir = env.actionCoords[action];
		// first row, second col
		if (abs(stateDim.first - 1 - (state.first + actionDir.first)) < cellsfromGoal.first ||
			abs(stateDim.second - 1- (state.second + actionDir.second)) < cellsfromGoal.second) {
			possibleActions.push_back(action);
		}
	}

	std::random_device rand_dev;
	std::mt19937 generator(rand_dev());
	std::uniform_int_distribution<int>  distr(0, possibleActions.size() - 1);
	int index = distr(generator);
	return possibleActions.at(index);
}

void Agent::trainRBM(std::tuple<std::pair<int, int>, int, std::pair<int, int>, float, bool> t)
{
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

void Agent::reset()
{
	epsilon = 1.f;
	epsilonDecay = 0.99f;
	beta = 0.99f;
	gamma = 0.99f;
	for (int row = 0; row < stateDim.first; ++row) {
		for (int col = 0; col < stateDim.second; ++col) {
			for (int action = 0; action < actionDim.first; ++action) {
				Q[row][col][action] = 0;
			}
		}
	}
}

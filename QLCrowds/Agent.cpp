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
	if (randVal < 1 - epsilon) {
		auto actions_allowed = env.allowedActions();
		if (m_backTracking) {
			// Strip away backtracking actions
			if (actions_allowed.size() > 1) {
				bool actionRemoved = false;
				int actionToRemove;
				for (auto & action : actions_allowed) {
					std::pair<int, int> actionDir = env.actionCoords[action];
					std::pair<int, int> nextState(env.state.first + actionDir.first, env.state.second + actionDir.second);
					if (nextState.first == m_previousState.first && nextState.second == m_previousState.second) {
						actionToRemove = action;
						actionRemoved = true;
						break;
					}
				}
				if (actionRemoved) {
					actions_allowed.erase(std::remove(actions_allowed.begin(), actions_allowed.end(), actionToRemove), actions_allowed.end());
				}
			}
		}
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
	else {
		auto actions_allowed = env.allowedActions();
		if (m_backTracking) {
			if (actions_allowed.size() > 1) {
				bool actionRemoved = false;
				int actionToRemove;
				for (auto & action : actions_allowed) {
					std::pair<int, int> actionDir = env.actionCoords[action];
					std::pair<int, int> nextState(env.state.first + actionDir.first, env.state.second + actionDir.second);
					if (nextState.first == m_previousState.first && nextState.second == m_previousState.second) {
						actionToRemove = action;
						actionRemoved = true;
						break;
					}
				}
				if (actionRemoved) {
					actions_allowed.erase(std::remove(actions_allowed.begin(), actions_allowed.end(), actionToRemove), actions_allowed.end());
				}
			}
		}
		std::uniform_int_distribution<int>  distr(0, actions_allowed.size() - 1);
		int index = distr(generator);
		return actions_allowed.at(index);
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
	std::cout << "C: " << state.first << "," << state.second << " P: " << m_previousState.first << "," << m_previousState.second << std::endl;

	float & sa = Q[state.first][state.second][action];
	auto nextActions = Q[state_next.first][state_next.second];
	m_previousState = state;

	auto maxElement = *std::max_element(nextActions.begin(), nextActions.end());
	Q[state.first][state.second][action] += beta * (reward + gamma * maxElement - sa);
}

int Agent::getActionRBMBased(Environment & env)
{
	auto allowedActions = env.allowedActions();
	auto & state = env.state;
	auto & goals = env.getGoals();
	std::vector<int> possibleActions;
	std::pair<int, int> cellsfromGoal;
	std::pair<int, int> closestGoal = goals.at(0);
	cellsfromGoal.first = abs(goals.at(0).first - state.first);
	cellsfromGoal.second = abs(goals.at(0).second - state.second);
	int combinedCellDist = cellsfromGoal.first + cellsfromGoal.second;
	for (std::pair<int, int> & goal : goals) {
		int goalcellDist = abs(goal.first - state.first) + abs(goal.second - state.second);
		if (goalcellDist < combinedCellDist) {
			combinedCellDist = goalcellDist;
			cellsfromGoal = std::make_pair(abs(goal.first - state.first),abs(goal.second - state.second));
			closestGoal = goal;
		}
	}
	int actionSize = allowedActions.size();
	for (auto & action : allowedActions) {
		std::pair<int, int> nextState;
		auto actionDir = env.actionCoords[action];
		nextState.first = actionDir.first + state.first;
		nextState.second = actionDir.second + state.second;
		// first row, second col
		if (env.m_heatMap[nextState.first][nextState.second] <= 2) {
			possibleActions.push_back(action);
		}
	}
	std::vector<int> actionsToRemove;
	for (auto & action : possibleActions) {
		std::pair<int, int> nextState;
		auto actionDir = env.actionCoords[action];
		nextState.first = actionDir.first + state.first;
		nextState.second = actionDir.second + state.second;
		if (abs(closestGoal.first - nextState.first) + abs(closestGoal.second - nextState.second) > combinedCellDist) {
			actionsToRemove.push_back(action);
		}
	}
	if (actionsToRemove.size() != possibleActions.size()) {
		for (auto & action : actionsToRemove) {
			possibleActions.erase(std::remove(possibleActions.begin(), possibleActions.end(), action), possibleActions.end());
		}
	}

	std::random_device rand_dev;
	std::mt19937 generator(rand_dev());
	int index;
	std::uniform_int_distribution<int>  distr;
	if (!possibleActions.empty()) {
		distr = std::uniform_int_distribution<int>(0, possibleActions.size() - 1);
	}
	else {
		distr = std::uniform_int_distribution<int>(0, allowedActions.size() - 1);
	}
	index = distr(generator);
	if (!possibleActions.empty())
		return possibleActions.at(index);
	else
		return allowedActions.at(index);

	//// Get the allowed actions
	//std::vector<int> allowedActions = env.allowedActions();
	//auto & currentState = env.state;

	//// Get the vector of goal areas in the environment
	//auto & goals = env.getGoals();
	//std::vector<int> possibleActions;
	//for (auto & action : allowedActions) {
	//	std::pair<int, int> & actionDir = env.actionCoords[action];
	//	std::pair<int, int> nextState(currentState.first + actionDir.first, currentState.second + actionDir.second);
	//	bool backTracking = env.m_heatMap[nextState.first][nextState.second] >= 2;
	//	if (!backTracking) {
	//		
	//	}
	//}
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

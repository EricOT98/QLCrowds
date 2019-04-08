#include "Agent.h"
#include <random>
#include <map>

#include <algorithm>
#include <iterator>

Agent::Agent(Environment &env, SDL_Renderer * renderer) :
	m_env(env)
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
	m_backTracking = true;
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
		auto actions_allowed = env.allowedActions(m_currentState);
		if (m_backTracking) {
			if (actions_allowed.size() > 1) {
				bool actionRemoved = false;
				int actionToRemove;
				for (auto & action : actions_allowed) {
					std::pair<int, int> actionDir = env.actionCoords[action];
					std::pair<int, int> nextState(m_currentState.first + actionDir.first, m_currentState.second + actionDir.second);
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
	else {

		auto actions_allowed = env.allowedActions(m_currentState);
		if (m_backTracking) {
			// Strip away backtracking actions
			if (actions_allowed.size() > 1) {
				bool actionRemoved = false;
				int actionToRemove;
				for (auto & action : actions_allowed) {
					std::pair<int, int> actionDir = env.actionCoords[action];
					std::pair<int, int> nextState(m_currentState.first + actionDir.first, m_currentState.second + actionDir.second);
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
		auto actionValues = Q[m_currentState.first][m_currentState.second];

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
	if (reward == 100) {
		std::cout << "";
	}
	Q[state.first][state.second][action] += beta * (reward + gamma * maxElement - sa);
}

int Agent::getActionRBMBased(Environment & env)
{
	auto allowedActions = env.allowedActions(m_currentState);
	auto & goals = env.getGoals();
	std::pair<int, int> cellsfromGoal;
	std::pair<int, int> closestGoal = goals.at(0);
	cellsfromGoal.first = abs(goals.at(0).first - m_currentState.first);
	cellsfromGoal.second = abs(goals.at(0).second - m_currentState.second);
	int combinedCellDist = cellsfromGoal.first + cellsfromGoal.second;
	for (std::pair<int, int> & goal : goals) {
		int goalcellDist = abs(goal.first - m_currentState.first) + abs(goal.second - m_currentState.second);
		if (goalcellDist < combinedCellDist) {
			combinedCellDist = goalcellDist;
			cellsfromGoal = std::make_pair(abs(goal.first - m_currentState.first), abs(goal.second - m_currentState.second));
			closestGoal = goal;
		}
	}
	std::vector<std::pair<int, int>> closestGoals;
	for (auto & goal : goals) {
		if (abs(goal.first - m_currentState.first) + abs(goal.second - m_currentState.second) == combinedCellDist) {
			closestGoals.push_back(goal);
		}
	}
	std::vector<int> actionsToRemove;
	for (auto & action : allowedActions) {
		std::pair<int, int> nextState;
		auto actionDir = env.actionCoords[action];
		nextState.first = actionDir.first + m_currentState.first;
		nextState.second = actionDir.second + m_currentState.second;
		bool progresses = false;
		for (auto & goal : closestGoals) {
			if (abs(goal.first - nextState.first) + abs(goal.second - nextState.second) < combinedCellDist)
				progresses = true;
		}
		if (!progresses) {
			actionsToRemove.push_back(action);
		}
	}
	if (actionsToRemove.size() != allowedActions.size()) {
		for (auto & action : actionsToRemove) {
			allowedActions.erase(std::remove(allowedActions.begin(), allowedActions.end(), action), allowedActions.end());
		}
	}

	std::random_device rand_dev;
	std::mt19937 generator(rand_dev());

	std::uniform_int_distribution<int>  distr;
	distr = std::uniform_int_distribution<int>(0, allowedActions.size() - 1);
	int index = distr(generator);
	if (std::find(allowedActions.begin(), allowedActions.end(), env.action_dict["none"]) != allowedActions.end()) {
		std::cout << "None action found" << std::endl;
	}
	return allowedActions.at(index);
}

int Agent::getMultiAgentAction(Environment & env)
{
	return 0;
}

int Agent::getMultiAgentActionRBM(Environment & env, int currentIter, const int maxIters)
{
	auto allowedActions = env.allowedActions(m_currentState);
	auto & goals = env.getGoals();

	std::pair<int, int> cellsfromGoal;
	std::pair<int, int> closestGoal = goals.at(0);
	cellsfromGoal.first = abs(goals.at(0).first - m_currentState.first);
	cellsfromGoal.second = abs(goals.at(0).second - m_currentState.second);
	int combinedCellDist = cellsfromGoal.first + cellsfromGoal.second;
	for (std::pair<int, int> & goal : goals) {
		int goalcellDist = abs(goal.first - m_currentState.first) + abs(goal.second - m_currentState.second);
		if (goalcellDist < combinedCellDist) {
			combinedCellDist = goalcellDist;
			cellsfromGoal = std::make_pair(abs(goal.first - m_currentState.first), abs(goal.second - m_currentState.second));
			closestGoal = goal;
		}
	}

	// If you must go to goal
	//if (combinedCellDist >= maxIters - 1 - currentIter) {
	std::vector<int> actionsToRemove;
	/*if (m_backTracking) {
		if (allowedActions.size() > 1) {
			bool actionRemoved = false;
			int actionToRemove;
			for (auto & action : allowedActions) {
				std::pair<int, int> actionDir = env.actionCoords[action];
				std::pair<int, int> nextState(m_currentState.first + actionDir.first, m_currentState.second + actionDir.second);
				if (nextState.first == m_previousState.first && nextState.second == m_previousState.second) {
					actionToRemove = action;
					actionRemoved = true;
					break;
				}
			}
			if (actionRemoved) {
				allowedActions.erase(std::remove(allowedActions.begin(), allowedActions.end(), actionToRemove), allowedActions.end());
			}
		}
	}*/
	for (auto & action : allowedActions) {
		std::pair<int, int> nextState;
		auto actionDir = env.actionCoords[action];
		nextState.first = actionDir.first + m_currentState.first;
		nextState.second = actionDir.second + m_currentState.second;
		if (abs(closestGoal.first - nextState.first) + abs(closestGoal.second - nextState.second) >= combinedCellDist) {
			actionsToRemove.push_back(action);
		}
	}
	if (actionsToRemove.size() != allowedActions.size()) {
		for (auto & action : actionsToRemove) {
			allowedActions.erase(std::remove(allowedActions.begin(), allowedActions.end(), action), allowedActions.end());
		}
	}
	//}
	//else { // If you can group
	//	std::pair<int, int> closestAgentState = env.getClosestAgent(m_currentState);
	//	int agentDist = abs(closestAgentState.first - m_currentState.first) + abs(closestAgentState.second - m_currentState.second);
	//	std::vector<int> actionsToRemove;
	//	if (m_backTracking) {
	//		if (allowedActions.size() > 1) {
	//			bool actionRemoved = false;
	//			int actionToRemove;
	//			for (auto & action : allowedActions) {
	//				std::pair<int, int> actionDir = env.actionCoords[action];
	//				std::pair<int, int> nextState(m_currentState.first + actionDir.first, m_currentState.second + actionDir.second);
	//				if (nextState.first == m_previousState.first && nextState.second == m_previousState.second) {
	//					actionToRemove = action;
	//					actionRemoved = true;
	//					break;
	//				}
	//			}
	//			if (actionRemoved) {
	//				allowedActions.erase(std::remove(allowedActions.begin(), allowedActions.end(), actionToRemove), allowedActions.end());
	//			}
	//		}
	//	}
	//	for (auto & action : allowedActions) {
	//		std::pair<int, int> nextState;
	//		auto actionDir = env.actionCoords[action];
	//		nextState.first = actionDir.first + m_currentState.first;
	//		nextState.second = actionDir.second + m_currentState.second;
	//		if (abs(closestAgentState.first - nextState.first) + abs(closestAgentState.second - nextState.second) > agentDist) {
	//			actionsToRemove.push_back(action);
	//		}
	//	}
	//	if (actionsToRemove.size() != allowedActions.size()) {
	//		for (auto & action : actionsToRemove) {
	//			allowedActions.erase(std::remove(allowedActions.begin(), allowedActions.end(), action), allowedActions.end());
	//		}
	//	}
	std::random_device rand_dev;
	std::mt19937 generator(rand_dev());

	std::uniform_int_distribution<int>  distr;
	distr = std::uniform_int_distribution<int>(0, allowedActions.size() - 1);
	int index = distr(generator);
	return allowedActions.at(index);
}

void Agent::trainRBM(std::tuple<std::pair<int, int>, int, std::pair<int, int>, float, bool> t)
{
}

tiny_dnn::network<tiny_dnn::sequential> Agent::buildModel()
{
	using namespace tiny_dnn;
	using namespace tiny_dnn::activation;
	using namespace tiny_dnn::layers;
	network<sequential> test_nn;
	
	/*
	How we do the architecture of the network
	Input layer is 2 (state of agent position as 2 floats) + 
	2 * number of obstcales and goals combined (each input is 2 floats representing state

	*/

	// FC is equivalent of Keras dense
	
	test_nn
		<< fully_connected_layer(4, 3, true) << activation::relu()
		<< fully_connected_layer(3, 3, true) << activation::relu()
		<< fully_connected_layer(3, 5, true) << activation::softmax();
	for (auto & layer : test_nn) {
		std::cout << "Layer: " << std::endl;
		auto lw = layer->weights();
		if (!lw.empty()) {
			auto & w = *lw[0];
			auto & b = *lw[1];
			std::cout << "Weights ";
			for (auto & weight : w) {
				std::cout << weight << ", ";
			}
			std::cout << std::endl;

			std::cout << "Biases: ";
			for (auto & bias : b) {
				std::cout << bias << ", ";
			}
		}
		std::cout << std::endl;
	}
	// Use mse for loss
	// Use adam for optimser
	
	return test_nn;
}

void Agent::updateTargetModel()
{

}

void Agent::replayMemory(AgentMemoryBatch memory)
{
	m_memory.push_back(memory);
	if (epsilon > 0.1)
		epsilon *= epsilonDecay;
}

/// <summary>
/// Pick random samples from within replay memeory in batch size to train the model
/// </summary>
void Agent::trainReplay()
{
	if (m_memory.size() < trainStart) {
		// No full batch preset so do nothing
	}
	else {
		int bs = std::min(batchSize, (int)m_memory.size());
		auto memoryCopy = m_memory;
		std::random_shuffle(memoryCopy.begin(), memoryCopy.end());
		std::vector<AgentMemoryBatch> miniBatch;
		for (int i = 0; i < bs; ++i) {
			miniBatch.push_back(memoryCopy.at(i));
		}

		for (int i = 0; i < bs; ++i) {
			auto & currentMemory = miniBatch[i];
			std::vector<float> states;
			states.push_back(currentMemory.state.first);
			states.push_back(currentMemory.state.second);
			auto goal = m_env.getGoals().at(0);
			states.push_back(goal.first);
			states.push_back(goal.second);
			tiny_dnn::vec_t target = model.predict(states);
			if (m_done)
				target[currentMemory.action] = currentMemory.reward;
			else {
				//Q[state.first][state.second][action] += beta * (reward + gamma * maxElement - sa);
				states.clear();
				states.push_back(currentMemory.nextState.first);
				states.push_back(currentMemory.nextState.second);
				states.push_back(goal.first);
				states.push_back(goal.second);
				auto predicted = model.predict(states);
				//auto maxPred = getBestActions(predicted);
				
				/*target = currentBatch.reward + ((tiny_dnn::float_t)gamma * model.predict(currentBatch.nextState));*/
			}
		}
	}
}

tiny_dnn::vec_t Agent::getBestActions(tiny_dnn::vec_t actions)
{
	auto maxElement = std::max_element(actions.begin(), actions.end());
	tiny_dnn::vec_t maxActions;
	/*for (auto & action : actions) {
		if (action == *maxElement)
			maxActions.push_back(action);
	}*/
	return maxActions;
}

void Agent::displayGreedyPolicy(Environment & env)
{

	std::vector<std::string> action_dict = { "u", "r", "d", "l", "n" };

	std::vector<std::vector<std::string>> greedyPolicy;
	greedyPolicy.resize(stateDim.first);
	for (int row = 0; row < stateDim.first; ++row) {
		std::cout << "[";
		greedyPolicy.at(row).resize(stateDim.second);
		for (int col = 0; col < stateDim.second; ++col) {
			if (env.m_tileFlags[row][col] & QLCTileGoal) {
				greedyPolicy[row][col] = "g";
			}
			else if (env.m_tileFlags[row][col] & QLCTileObstacle) {
				greedyPolicy[row][col] = "o";
			}
			else {
				auto actions = Q[row][col];
				greedyPolicy[row][col] = *std::max_element(actions.begin(), actions.end());
				float best = actions.at(0);
				int selected = 0;
				for (int i = 1; i < actions.size(); ++i) {
					if (actions[i] > best) {
						best = actions[i];
						selected = i;
					}
				}
				greedyPolicy[row][col] = action_dict.at(selected);
			}
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

void Agent::setOrientation(int action)
{
	switch (action)
	{
	case 0:
		m_angle = 90;
		break;
	case 1:
		m_angle = 0;
		break;
	case 2:
		m_angle = 270;
		break;
	case 3:
		m_angle = 180;
		break;
	default:
		m_angle = 0;
		break;
	}
}

void Agent::render(SDL_Renderer & renderer)
{
	m_sprite.render(&renderer, m_angle);
}

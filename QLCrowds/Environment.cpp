#include "Environment.h"

Environment::Environment()
{
	stateDim = std::make_pair(ySize, xSize);
	actionDim = std::make_pair(4, 0);

	action_dict.insert(std::make_pair<std::string, int>("up", 0));
	action_dict.insert(std::make_pair<std::string, int>("right", 1));
	action_dict.insert(std::make_pair<std::string, int>("down", 2));
	action_dict.insert(std::make_pair<std::string, int>("left", 3));
	buildRewards();
}

Environment::~Environment()
{
}

void Environment::buildRewards()
{
	float rGoal = 100;
	float rNonGoal = -0.1f;
	R.resize(stateDim.first);
	for (int row = 0; row < ySize; ++row) {
		R.at(row).resize(stateDim.second);
		for (int col = 0; col < xSize; ++col) {
			R.at(row).at(col).resize(actionDim.first);
			for (int a = 0; a < actionDim.first; ++a) {
				R[row][col][a] = rNonGoal;
			}
		}
	}

	R[ySize - 1][xSize - 1][action_dict.at("down")] = rGoal;
	R[ySize - 1][xSize - 1][action_dict.at("right")] = rGoal;
}

std::tuple<std::pair<int, int>, float, bool> Environment::step(int action)
{
	std::pair<int, int> next_state(
		state.first + actionCoords[action][0],
		state.second + actionCoords[action][1]);

	float reward = R[state.first][state.second][action];

	bool done = (next_state.second == ySize - 1) && (next_state.first == xSize - 1);
	state = next_state;
	return std::make_tuple(next_state, reward, done);
}

std::pair<int, int> Environment::reset()
{
	state.first = 0;
	state.second = 0;
	return state;
}

std::vector<int> Environment::allowedActions()
{
	std::vector<int> allowed;

	int row = state.first;
	int col = state.second;

	if (row > 0)
		allowed.push_back(action_dict.at("up"));
	if (row < ySize - 1)
		allowed.push_back(action_dict.at("down"));
	if (col > 0)
		allowed.push_back(action_dict.at("left"));
	if (col < xSize - 1)
		allowed.push_back(action_dict.at("right"));

	return allowed;
}

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
	generateGridLines();
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

	R[ySize - 2][xSize - 1][action_dict.at("down")] = rGoal;
	R[ySize - 1][xSize - 2][action_dict.at("right")] = rGoal;
}

std::tuple<std::pair<int, int>, float, bool> Environment::step(int action)
{
	std::pair<int, int> next_state(
		state.first + actionCoords[action].first,
		state.second + actionCoords[action].second);

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
		allowed.push_back(action_dict["up"]);
	if (row < ySize - 1)
		allowed.push_back(action_dict["down"]);
	if (col > 0)
		allowed.push_back(action_dict["left"]);
	if (col < xSize - 1)
		allowed.push_back(action_dict["right"]);

	return allowed;
}

void Environment::generateGridLines()
{
	gridLines.clear();
	int endPosX = gridPosX + (stateDim.second * cellW);
	for (int row = 0; row < stateDim.first + 1; ++row) {
		int yPos = gridPosY + (row * cellH);
		Line l;
		l.x1 = gridPosX;
		l.y1 = yPos;
		l.x2 = endPosX;
		l.y2 = yPos;
		gridLines.push_back(l);
	}
	int endPosY = gridPosY + (stateDim.first * cellH);
	for (int col = 0; col < stateDim.second + 1; ++col) {
		int xPos = gridPosX + (col * cellW);
		Line l;
		l.x1 = xPos;
		l.y1 = gridPosY;
		l.x2 = xPos;
		l.y2 = endPosY;
		gridLines.push_back(l);
	}
}

void Environment::render(SDL_Renderer & renderer)
{
	SDL_SetRenderDrawColor(&renderer, 255, 0, 0, 255);
	for (auto & line : gridLines) {
		SDL_RenderDrawLine(&renderer, line.x1, line.y1, line.x2, line.y2);
	}
	SDL_SetRenderDrawColor(&renderer, 0, 0, 0, 255);
}

void Environment::resizeGridTo(int x, int y, int width, int height)
{
	gridPosX = x;
	gridPosY = y;
	cellW = width / stateDim.second;
	cellH = height / stateDim.first;
	generateGridLines();
}

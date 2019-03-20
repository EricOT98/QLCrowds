#include "Environment.h"
#include <limits>
#include <algorithm>

/// <summary>
/// Initializes a new instance of the <see cref="Environment"/> class.
/// </summary>
Environment::Environment()
{
	stateDim = std::make_pair(ySize, xSize);
	actionDim = std::make_pair(4, 0);

	action_dict.insert(std::make_pair<std::string, int>("up", 0));
	action_dict.insert(std::make_pair<std::string, int>("right", 1));
	action_dict.insert(std::make_pair<std::string, int>("down", 2));
	action_dict.insert(std::make_pair<std::string, int>("left", 3));
	m_heatMap.resize(stateDim.first);
	for (int row = 0; row < stateDim.first; ++row) {
		m_heatMap.at(row).resize(stateDim.second);
		for (int col = 0; col < stateDim.second; ++col) {
			m_heatMap[row][col] = 0;
		}
	}
	initFlags();
	buildRewards();
	generateGridLines();
	m_tileFlags[1][0] |= QLCTileObstacle;
}

/// <summary>
/// Finalizes an instance of the <see cref="Environment"/> class.
/// </summary>
Environment::~Environment()
{
}

/// <summary>
/// Builds the rewards.
/// </summary>
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

	/*R[ySize - 2][xSize - 1][action_dict.at("down")] = rGoal;
	R[ySize - 1][xSize - 2][action_dict.at("right")] = rGoal;*/
}

/// <summary>
/// Steps the specified action.
/// </summary>
/// <param name="action">The action.</param>
/// <returns></returns>
std::tuple<std::pair<int, int>, float, bool> Environment::step(int action, std::pair<int, int> & state)
{
	m_heatMap[state.first][state.second] += 1;
	std::pair<int, int> next_state(
		state.first + actionCoords[action].first,
		state.second + actionCoords[action].second);

	float reward = R[state.first][state.second][action];
	bool done = m_tileFlags[next_state.first][next_state.second] & QLCTileGoal;
	return std::make_tuple(next_state, reward, done);
}

/// <summary>
/// Resets this instance.
/// </summary>
/// <returns></returns>
void Environment::reset()
{
	for (auto & state : m_states) {
		state.first = 0;
		state.second = 0;
	}
}

/// <summary>
/// Alloweds the actions.
/// </summary>
/// <returns></returns>
std::vector<int> Environment::allowedActions(const std::pair<int, int> & state)
{
	std::vector<int> allowed;
	int row = state.first;
	int col = state.second;

	if (row > 0) {
		auto & upFlags = m_tileFlags[row - 1][col];
		if (!(upFlags & QLCTileObstacle))
			allowed.push_back(action_dict["up"]);
	}
	if (row < ySize - 1) {
		auto & downFlags = m_tileFlags[row + 1][col];
		if (!(downFlags & QLCTileObstacle))
			allowed.push_back(action_dict["down"]);
	}
	if (col > 0) {
		auto & leftFlags = m_tileFlags[row][col - 1];
		if (!(leftFlags & QLCTileObstacle))
			allowed.push_back(action_dict["left"]);
	}
	if (col < xSize - 1) {
		auto & rightFlags = m_tileFlags[row][col + 1];
		if (!(rightFlags & QLCTileObstacle))
			allowed.push_back(action_dict["right"]);
	}

	return allowed;
}

/// <summary>
/// Generates the grid lines.
/// </summary>
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

/// <summary>
/// Renders the specified renderer.
/// </summary>
/// <param name="renderer">The renderer.</param>
void Environment::render(SDL_Renderer & renderer)
{
	SDL_SetRenderDrawColor(&renderer, 255, 0, 0, 255);
	for (auto & line : gridLines) {
		SDL_RenderDrawLine(&renderer, line.x1, line.y1, line.x2, line.y2);
	}
	SDL_Rect rect;
	rect.w = cellW - 1;
	rect.h = cellH - 1;

	for (int row = 0; row < stateDim.first; ++row) {
		rect.y = gridPosY + (row * cellH) + 1;
		for (int col = 0; col < stateDim.second; ++col) {
			rect.x = gridPosX + (col * cellW) + 1;
			int alpha = (m_heatMap[row][col] / (float)m_largestHeatMapVal) * 255;
			if (alpha > 255)
				alpha = 255;
			if (m_tileFlags[row][col] & QLCTileGoal)
				SDL_SetRenderDrawColor(&renderer, 0, 255, 0, 255);
			else if (m_tileFlags[row][col] & QLCTileObstacle)
				SDL_SetRenderDrawColor(&renderer, 0, 0, 255, 255);
			else
				SDL_SetRenderDrawColor(&renderer, 214, 79, 29, alpha);
			SDL_RenderFillRect(&renderer, &rect);
			SDL_SetRenderDrawColor(&renderer, 0, 0, 0, 255);
		}
	}
	SDL_SetRenderDrawColor(&renderer, 0, 0, 0, 255);
}

/// <summary>
/// Resizes the grid to.
/// </summary>
/// <param name="x">The x.</param>
/// <param name="y">The y.</param>
/// <param name="width">The width.</param>
/// <param name="height">The height.</param>
void Environment::resizeGridTo(int x, int y, int width, int height)
{
	gridPosX = x;
	gridPosY = y;
	cellW = width / stateDim.second;
	cellH = height / stateDim.first;
	generateGridLines();
}

void Environment::createHeatmapVals()
{
	int largestElement = m_heatMap[0][0];
	for (int row = 0; row < stateDim.first; ++row) {
		for (int col = 0; col < stateDim.second; ++col) {
			if (m_heatMap[row][col] > largestElement) {
				largestElement = m_heatMap[row][col];
			}
		}
	}
	m_largestHeatMapVal = largestElement;
}

/// <summary>
/// Adds the obstacle.
/// </summary>
/// <param name="row">The row.</param>
/// <param name="col">The col.</param>
void Environment::addObstacle(int row, int col)
{
	m_tileFlags[row][col] ^= QLCTileObstacle;
}

void Environment::addGoal(int row, int col)
{
	float rGoal = 100;
	float rNonGoal = -0.1f;
	m_tileFlags[row][col] ^= QLCTileGoal;
	bool active = m_tileFlags[row][col] & QLCTileGoal;
	if (active) {
		m_goals.push_back(std::make_pair(row, col));
	}
	else {
		m_goals.erase(std::remove(m_goals.begin(), m_goals.end(), std::make_pair(row, col)), m_goals.end());
	}
	int goalValue = active ? rGoal : rNonGoal;
	if (row > 0) {
		auto & upFlags = m_tileFlags[row - 1][col];
		if (!(upFlags & QLCTileObstacle))
			R[row - 1][col][action_dict["down"]] = goalValue;
	}
	if (row < ySize - 1) {
		auto & downFlags = m_tileFlags[row + 1][col];
		if (!(downFlags & QLCTileObstacle))
			R[row + 1][col][action_dict["up"]] = goalValue;
	}
	if (col > 0) {
		auto & leftFlags = m_tileFlags[row][col - 1];
		if (!(leftFlags & QLCTileObstacle))
			R[row][col - 1][action_dict["right"]] = goalValue;
	}
	if (col < xSize - 1) {
		auto & rightFlags = m_tileFlags[row][col + 1];
		if (!(rightFlags & QLCTileObstacle))
			R[row][col + 1][action_dict["left"]] = goalValue;
	}
}

std::vector<std::pair<int, int>> & Environment::getGoals()
{
	return m_goals;
}

/// <summary>
/// Resets the flags.
/// </summary>
void Environment::resetFlags()
{
	for (auto & row : m_tileFlags) {
		for (auto & col : row) {
			col = QLCTileEMPTY;
		}
	}
}

/// <summary>
/// Initializes the flags.
/// </summary>
void Environment::initFlags()
{
	m_tileFlags.resize(stateDim.first);
	for (int row = 0; row < stateDim.first; ++row) {
		m_tileFlags[row].resize(stateDim.second);
		for (int col = 0; col < stateDim.second; ++col) {
			m_tileFlags[row][col] = QLCTileEMPTY;
		}
	}
}

void Environment::clearHeatMap()
{
	for (int row = 0; row < stateDim.first; ++row) {
		for (int col = 0; col < stateDim.second; ++col) {
			m_heatMap[row][col] = 0;
		}
	}
}

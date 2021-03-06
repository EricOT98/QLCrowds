#include "Environment.h"
#include <limits>
#include <algorithm>
#include <iostream>
#include <math.h>

/// <summary>
/// Initializes a new instance of the <see cref="Environment"/> class.
/// </summary>
Environment::Environment()
{

	action_dict.insert(std::make_pair<std::string, int>("up", 0));
	action_dict.insert(std::make_pair<std::string, int>("right", 1));
	action_dict.insert(std::make_pair<std::string, int>("down", 2));
	action_dict.insert(std::make_pair<std::string, int>("left", 3));
	action_dict.insert(std::make_pair<std::string, int>("none", 4));
	init(xSize, ySize);
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
	R.resize(m_stateDim.first);
	for (int row = 0; row < ySize; ++row) {
		R.at(row).resize(m_stateDim.second);
		for (int col = 0; col < xSize; ++col) {
			R.at(row).at(col).resize(m_actionDim.first);
			for (int a = 0; a < m_actionDim.first; ++a) {
				R[row][col][a] = rNonGoal;
			}
		}
	}
}

/// <summary>
/// Steps the environment one step forward in the simulation.
/// Calcualates the next state of the agent and returns the reward value, next state and completion values.
/// </summary>
/// <param name="action">The action.</param>
/// <returns>A tuple of the agents next state , reward and completion values</returns>
std::tuple<std::pair<int, int>, float, bool> Environment::step(int action, std::pair<int, int> & state)
{
	m_heatMap[state.first][state.second] += 1;
	std::pair<int, int> next_state(
		state.first + actionCoords[action].first,
		state.second + actionCoords[action].second);

	auto closestagent = getClosestAgent(state);
	float reward = R[state.first][state.second][action] - (std::abs(next_state.first - state.first) + std::abs(next_state.second - state.second));
	bool done = m_tileFlags[next_state.first][next_state.second] & QLCTileGoal;

	if (!done && m_tileFlags[next_state.first][next_state.second] & QLCContainsAgent) {
		if (next_state.first != state.first && next_state.second != state.second)
			reward = -10.f;
	}
	return std::make_tuple(next_state, reward, done);
}

/// <summary>
/// Steps the environment in a JAQL simulation by taking in the action coupling of agents as a vector of actions and the state coupling
/// as a vector of states. These values are then used to make the combined next state and generate a reward value based on the combined agent actions.
/// </summary>
/// <param name="actions">The actions.</param>
/// <param name="states">The states.</param>
/// <returns></returns>
std::tuple<std::vector<State>, float, bool> Environment::stepJAQL(std::vector<int> & actions, std::vector<State>& states)
{
	for (auto & state : m_states) {
		m_heatMap[state.first][state.second] += 1;
	}
	std::vector<State> nextStates;
	for (int i = 0; i < states.size(); ++i) {
		auto & state = states.at(i);
		auto & action = actions.at(i);
		nextStates.push_back(
			std::make_pair(
				state.first + actionCoords[action].first,
				state.second + actionCoords[action].second
			)
		);
	}
	float reward = 0;
	bool done = false;
	for (auto & next_state : nextStates) {
		if (m_tileFlags[next_state.first][next_state.second] & QLCTileGoal) {
			reward += 100;
			done = true;
			//break;
		}
		else if (m_tileFlags[next_state.first][next_state.second] & QLCContainsAgent) {
			reward -= 10;
		}
		else {
			reward -= 0.1f;
		}
	}
	
	return std::make_tuple(nextStates, reward, done);
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
	if (row < m_stateDim.first - 1) {
		auto & downFlags = m_tileFlags[row + 1][col];
		if (!(downFlags & QLCTileObstacle))
			allowed.push_back(action_dict["down"]);
	}
	if (col > 0) {
		auto & leftFlags = m_tileFlags[row][col - 1];
		if (!(leftFlags & QLCTileObstacle))
			allowed.push_back(action_dict["left"]);
	}
	if (col < m_stateDim.second - 1) {
		auto & rightFlags = m_tileFlags[row][col + 1];
		if (!(rightFlags & QLCTileObstacle))
			allowed.push_back(action_dict["right"]);
	}
	allowed.push_back(action_dict["none"]);

	return allowed;
}

std::pair<int, int>Environment::getClosestAgent(const std::pair<int, int>& state)
{
	std::pair<int, int> closestAgentState;

	// Iterate over all spaces if it contains an agent compare distance
	for (int row = 0; row < m_stateDim.first; ++row) {
		for (int col = 0; col < m_stateDim.second; ++col) {
			if (row != state.first && col != state.second) {
				if (m_tileFlags[row][col] & QLCContainsAgent) {
					if (closestAgentState.first && closestAgentState.second) {
						int combinedCellDist = std::abs(col - state.second) + std::abs(row - state.first);
						if (combinedCellDist < std::abs(closestAgentState.second - state.second) + std::abs(closestAgentState.first - state.first))
							closestAgentState = std::make_pair(row, col);
					}
					else
						closestAgentState = std::make_pair(row, col);
				}
			}
		}
	}
	auto allowed_actions = allowedActions(state);
	return closestAgentState;
}

/// <summary>
/// Generates the grid lines.
/// </summary>
void Environment::generateGridLines()
{
	cellW = floor(width / (float)m_stateDim.second);
	cellH = floor(height / (float)m_stateDim.first);
	m_gridLines.clear();
	int endPosX = gridPosX + (m_stateDim.second * cellW);
	for (int row = 0; row <= m_stateDim.first; ++row) {
		int yPos = gridPosY + (row * cellH);
		Line l;
		l.x1 = gridPosX;
		l.y1 = yPos;
		l.x2 = endPosX;
		l.y2 = yPos;
		m_gridLines.push_back(l);
	}
	int endPosY = gridPosY + (m_stateDim.first * cellH);
	for (int col = 0; col <= m_stateDim.second; ++col) {
		int xPos = gridPosX + (col * cellW);
		Line l;
		l.x1 = xPos;
		l.y1 = gridPosY;
		l.x2 = xPos;
		l.y2 = endPosY;
		m_gridLines.push_back(l);
	}
}

/// <summary>
/// Renders the grid based environment using the specified renderer.
/// </summary>
/// <param name="renderer">The renderer.</param>
void Environment::render(SDL_Renderer & renderer)
{
	SDL_SetRenderDrawColor(&renderer, 255, 0, 0, 255);
	for (auto & line : m_gridLines) {
		SDL_RenderDrawLine(&renderer, line.x1, line.y1, line.x2, line.y2);
	}
	SDL_Rect rect;
	rect.w = cellW - 1;
	rect.h = cellH - 1;

	for (int row = 0; row < m_stateDim.first; ++row) {
		rect.y = gridPosY + (row * cellH) + 1;
		for (int col = 0; col < m_stateDim.second; ++col) {
			rect.x = gridPosX + (col * cellW) + 1;
			SDL_Color colour = { 0,0,0, 255 };
			if (m_tileFlags[row][col] & QLCTileGoal)
				colour = { 0, 255, 0, 255 };
			else if (m_tileFlags[row][col] & QLCTileObstacle)
				colour = { 0, 0, 255, 255 };
			else if (m_tileFlags[row][col] & QLCVisited)
				colour = { 255, 0,0, 255 };
			else {
				if (m_largestHeatMapVal != 0) {
					Uint8 alpha = (m_heatMap[row][col] / (float)m_largestHeatMapVal) * 255;
					if (alpha > 255)
						alpha = 255;
					colour = { 214, 79, 29, alpha };
				}
			}
			SDL_SetRenderDrawColor(&renderer, colour.r, colour.g, colour.b, colour.a);
			SDL_RenderFillRect(&renderer, &rect);
		}
	}
	SDL_SetRenderDrawColor(&renderer, 0, 0, 0, 255);
}

/// <summary>
/// Resizes the grid to the given parameters and generates the new grid lines
/// </summary>
/// <param name="x">The x.</param>
/// <param name="y">The y.</param>
/// <param name="width">The width.</param>
/// <param name="height">The height.</param>
void Environment::resizeGridTo(int x, int y, int w, int h)
{
	gridPosX = x;
	gridPosY = y;
	width = w;
	height = h;
	generateGridLines();
}

/// <summary>
/// Get the largest value from the heatmap for use in displaying the heatmap gradient
/// </summary>
void Environment::createHeatmapVals()
{
	int largestElement = m_heatMap[0][0];
	for (int row = 0; row < m_stateDim.first; ++row) {
		for (int col = 0; col < m_stateDim.second; ++col) {
			if (m_heatMap[row][col] > largestElement) {
				largestElement = m_heatMap[row][col];
			}
		}
	}
	m_largestHeatMapVal = largestElement;
}

/// <summary>
/// Add an obstacle into the grid by toggling the active bit in the flag.
/// </summary>
/// <param name="row">The row.</param>
/// <param name="col">The col.</param>
void Environment::addObstacle(int row, int col)
{
	m_tileFlags[row][col] ^= QLCTileObstacle;
}

/// <summary>
/// Adds a goal to the grid and poulates the reward values of neighboring cells actions accordingly.
/// </summary>
/// <param name="row">The row.</param>
/// <param name="col">The col.</param>
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

/// <summary>
/// Return the grid position of all goals in the environment
/// </summary>
/// <returns>A vector of goal grid positions (a pair of ints)</returns>
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
	m_tileFlags.resize(m_stateDim.first);
	for (int row = 0; row < m_stateDim.first; ++row) {
		m_tileFlags[row].resize(m_stateDim.second);
		for (int col = 0; col < m_stateDim.second; ++col) {
			m_tileFlags[row][col] |= QLCTileEMPTY;
		}
	}
}

/// <summary>
/// Clears the heat map values.
/// </summary>
void Environment::clearHeatMap()
{
	for (int row = 0; row < m_stateDim.first; ++row) {
		for (int col = 0; col < m_stateDim.second; ++col) {
			m_heatMap[row][col] = 0;
		}
	}
}

/// <summary>
/// Initializes the environment to the specified dimensions
/// Reinitializes the grid flags and rewards
/// </summary>
/// <param name="x">The number of colimns</param>
/// <param name="y">The number of rows.</param>
void Environment::init(int x, int y)
{
	m_stateDim = std::make_pair(y, x);
	m_actionDim = std::make_pair(5, 0);
	m_heatMap.clear();
	m_heatMap.resize(m_stateDim.first);
	for (int row = 0; row < m_stateDim.first; ++row) {
		m_heatMap.at(row).resize(m_stateDim.second);
		for (int col = 0; col < m_stateDim.second; ++col) {
			m_heatMap[row][col] = 0;
		}
	}
	initFlags();
	buildRewards();
	generateGridLines();
}

/// <summary>
/// Sets the grid tile flag to contain an agent if found and empty the previous tile flag
/// </summary>
/// <param name="p">The p.</param>
/// <param name="c">The c.</param>
void Environment::setAgentFlags(std::pair<int, int> p, std::pair<int, int> c)
{
	auto & prev = m_tileFlags[p.first][p.second];
	auto & curr = m_tileFlags[c.first][c.second];
	prev = QLCTileEMPTY;
	auto pred = [&c](std::pair<int, int> & g) {
		return g.first == c.first && g.second == c.second;
	};
	if (std::find_if(m_goals.begin(), m_goals.end(), pred) == m_goals.end()) {
		curr |= QLCContainsAgent;
	}
}

/// <summary>
/// Returns a vector of all spawnable positions for agents on the grid
/// </summary>
/// <returns>All spawnable grid positions available for an agent</returns>
std::vector<std::pair<int, int>> Environment::getSpawnablePoint()
{
	std::vector<std::pair<int, int>> statesToCheck;
	for (int row = 0; row < m_stateDim.first; ++row) {
		for (int col = 0; col < m_stateDim.second; ++col) {
			auto & flags = m_tileFlags[row][col];
			if (!(flags& QLCTileGoal || flags & QLCTileObstacle || flags & QLCContainsAgent))
				statesToCheck.push_back(std::make_pair(row, col));
		}
	}
	return statesToCheck;
}

/// <summary>
/// Gets the number of obstacles in the grid (for use with dqn mainly).
/// </summary>
/// <returns>Number of obstacles in the grid</returns>
int Environment::getNumberOfObstacles()
{
	int numObstacles = 0;
	for (int row = 0; row < m_stateDim.first; ++row) {
		for (int col = 0; col < m_stateDim.second; ++col) {
			if (m_tileFlags[row][col] & QLCTileObstacle)
				numObstacles++;
		}
	}
	return numObstacles;
}

/// <summary>
/// Gets the obstacle in the grids.
/// </summary>
/// <returns>A vector of grid positions of all obstacles in the grid</returns>
std::vector<std::pair<int, int>> Environment::getObstacles()
{
	std::vector<std::pair<int, int>> obstacles;
	for (int row = 0; row < m_stateDim.first; ++row) {
		for (int col = 0; col < m_stateDim.second; ++col) {
			if (m_tileFlags[row][col] & QLCTileObstacle)
				obstacles.push_back(std::make_pair(row, col));
		}
	}
	return obstacles;
}

std::pair<int, int> Environment::getStateDim()
{
	return m_stateDim;
}

std::pair<int, int> Environment::getActionDim()
{
	return m_actionDim;
}

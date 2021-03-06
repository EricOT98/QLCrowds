#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <vector>
#include <map>
#include <tuple>
#include <SDL.h>

typedef int QLCTileFlags;
 /// <summary>
/// Flags for representing a tiles information and varying states
/// </summary>
enum QLCTileFlags_ {
	QLCTileEMPTY = 0,
	QLCTileObstacle = 1 << 0,
	QLCTileGoal = 1 << 1,
	QLCContainsAgent = 1 << 2,
	QLCVisited = 1 << 3
};
typedef std::pair<int, int> State;
/// <summary>
/// A class to represent a gridworld environment for ML agent simulations
/// Utilises discrete spaces.
/// </summary>
class Environment {
public:
	struct Line;
	// Member variables
	int xSize = 8;
	int ySize = 8;
	int width = 0;
	int height = 0;

	// ML related members
	std::vector<std::pair<int, int>> m_states;
	std::map<std::string, int> action_dict;
	std::pair<int, int> actionCoords[5] = { {-1, 0}, { 0, 1}, {1, 0}, {0, -1}, {0, 0} };
	std::vector<std::vector<std::vector<float>>> R;

	// Tile Info
	std::vector<std::vector<int>> m_tileFlags;
	// Display member vars
	int gridPosX = 0;
	int gridPosY = 0;
	int cellW = 32;
	int cellH = 32;
	std::vector<std::vector<int>> m_heatMap;
	int m_largestHeatMapVal;

	// Member function
	Environment();
	~Environment();

	std::tuple<std::pair<int, int>, float, bool> step(int action, std::pair<int, int> & state);
	std::tuple<std::vector<State>, float, bool> stepJAQL(std::vector<int> & actions, std::vector<State> & states);
	void reset();
	std::vector<int> allowedActions(const std::pair<int, int> & state);
	std::pair<int, int> getClosestAgent(const std::pair<int, int> & state);

	// display functions
	void render(SDL_Renderer & renderer);
	void generateGridLines();
	void resizeGridTo(int x, int y, int width, int height);
	void createHeatmapVals();
	
	// Environment Modifications
	void addObstacle(int row = 0, int col = 0);
	void addGoal(int row = 0, int col = 0);
	std::vector<std::pair<int, int>> & getGoals();
	void resetFlags();
	void initFlags();
	void clearHeatMap();
	void init(int x, int y);
	void setAgentFlags(std::pair<int, int> p, std::pair<int, int> c);
	std::vector<std::pair<int, int>> getSpawnablePoint();
	int getNumberOfObstacles();
	std::vector<std::pair<int, int>> getObstacles();

	// Getters
	std::pair<int, int> getStateDim();
	std::pair<int, int> getActionDim();
protected:
	// State rep
	std::pair<int, int> m_stateDim;
	std::pair<int, int> m_actionDim;

	std::vector<Line> m_gridLines;
	void buildRewards();
	std::vector<std::pair<int, int>> m_goals;
	struct Line {
		int x1;
		int x2;
		int y1;
		int y2;
	};
};

#endif //!ENVIRONMENT_H
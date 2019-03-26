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

/// <summary>
/// A class to represent a gridworld environment for ML agent simulations
/// Utilises discrete spaces.
/// </summary>
class Environment {
public:
	struct Line;
	// Member variables
	int xSize = 64;
	int ySize = 64;

	std::pair<int, int> stateDim;
	std::pair<int, int> actionDim;

	// ML related members
	std::vector<std::pair<int, int>> m_states;
	std::map<std::string, int> action_dict;
	std::pair<int, int> actionCoords[4] = { {-1, 0}, { 0, 1}, {1, 0}, {0, -1} };
	std::vector<std::vector<std::vector<float>>> R;

	// Tile Info
	std::vector<std::vector<int>> m_tileFlags;

	// Display member vars
	int gridPosX = 0;
	int gridPosY = 0;
	int cellW = 32;
	int cellH = 32;
	std::vector<Line> gridLines;
	std::vector<std::vector<int>> m_heatMap;
	int m_largestHeatMapVal;

	// Member function
	Environment();
	~Environment();
	
	void buildRewards();
	std::tuple<std::pair<int, int>, float, bool> step(int action, std::pair<int, int> & state);
	void reset();
	std::vector<int> allowedActions(const std::pair<int, int> & state);

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

protected:
	std::vector<std::pair<int, int>> m_goals;
	struct Line {
		int x1;
		int x2;
		int y1;
		int y2;
	};
};

#endif //!ENVIRONMENT_H
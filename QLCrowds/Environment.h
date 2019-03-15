#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <vector>
#include <map>
#include <tuple>
#include <SDL.h>

struct Line {
	int x1;
	int x2;
	int y1;
	int y2;
};

class Environment {
public:
	static const int xSize = 64;
	static const int ySize = 64;
	Environment();
	~Environment();

	std::pair<int, int> stateDim;
	std::pair<int, int> actionDim;

	int grid[xSize][ySize];
	std::map<std::string, int> action_dict;
	std::pair<int, int> actionCoords[4] = {{-1, 0}, { 0, 1}, {1, 0}, {0, -1}};
	std::vector<std::vector<std::vector<float>>> R;

	void buildRewards();
	std::tuple<std::pair<int, int>, float, bool> step(int action);
	std::pair<int, int> reset();
	std::vector<int> allowedActions();
	std::pair<int, int> state;

	// display functions
	void generateGridLines();
	void render(SDL_Renderer & renderer);
	void resizeGridTo(int x, int y, int width, int height);
	int cellW = 32;
	int cellH = 32;
	std::vector<Line> gridLines;
	int gridPosX = 0;
	int gridPosY = 0;
	std::vector<std::vector<int>> m_heatMap;
};

#endif //!ENVIRONMENT_H
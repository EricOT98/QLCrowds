#pragma comment(lib,"SDL2.lib") 
#pragma comment(lib,"SDL2main.lib") 
#pragma comment(lib, "SDL2_image.lib")

#include <iostream>
#include <SDL.h>
#include "Game.h"
#include <iostream>
 
#include "Environment.h"
#include "Agent.h"

/// <summary>
/// Default sdl main to use
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
/// <returns></returns>
int main(int argc, char * argv[]) {

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		std::cout << "SDL initialization failed. SDL Error: " << SDL_GetError();
	}
	else
	{
		std::cout << "SDL initialization succeeded!";
	}
	Game game;
	game.run();
	return 0;
}
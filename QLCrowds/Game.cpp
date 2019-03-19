#include "Game.h"
#include <iostream>
#include <fstream>
#include <SDL_image.h>
#include "MathUtils.h"

Game::Game()
{
	m_window = NULL;
	m_renderer = NULL;

	//if (SDL_Init(SDL_INIT_VIDEO) < 0) {
	//	std::cout << "SDL failed to initialize, SDL_Error: " << SDL_GetError() << std::endl;
	//}
	//else {
	m_window = SDL_CreateWindow("SDL: Command Pattern", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_windowWidth, m_windowHeight, SDL_WINDOW_SHOWN);
	if (!m_window) {
		std::cout << "SDL window failed to create, SDL_Error: " << SDL_GetError() << std::endl;
	}
	else {
		m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		if (!m_renderer) {
			std::cout << "Failed to create renderer, SDL_Erro: " << SDL_GetError();
		}
		SDL_RenderSetLogicalSize(m_renderer, m_windowWidth, m_windowHeight);
		SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
		SDL_SetRenderDrawBlendMode(m_renderer, SDL_BlendMode::SDL_BLENDMODE_BLEND);
		//Fill the surface white
	}
	//}
	if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
	{
		printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
	}

	// ImGui exclusive
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGuiSDL::Initialize(m_renderer, m_windowWidth, m_windowHeight);

	ImGui_ImplSDL2_InitForOpenGL(m_window, nullptr);

	// Actual code init
	env.resizeGridTo(0, 0, 640, 360);
	agent = new Agent(env, m_renderer);
	current_item = items[0];
}

Game::~Game()
{
	ImGuiSDL::Deinitialize();
	ImGui_ImplSDL2_Shutdown();
	SDL_DestroyRenderer(m_renderer);
	SDL_DestroyWindow(m_window);
	ImGui::DestroyContext();
	SDL_Quit();
}

void Game::update(float deltaTime)
{
	//std::cout << "				Update" << std::endl;
	if (m_simulationStarted) {
		if (currentEpisode < m_episodes.size()) {
			lerping = true;
			auto & episode = m_episodes.at(currentEpisode);
			if (currentIteration < episode.size()) {
				if (currentPercent < lerpMax)
					currentPercent += lerpPercent;
				else {
					currentPercent = 1.0f;
					lerping = false;
				}
				auto & data = episode.at(currentIteration);
				auto & nextState = data.nextState;
				auto & state = data.state;
				int w = env.cellW;
				int h = env.cellH;
				int currentW = w * state.second;
				int currentH = h * state.first;
				int nextW = w * nextState.second;
				int nextH = h * nextState.first;
				agent->m_sprite.setPosition(mu::lerp(currentW, nextW, currentPercent), mu::lerp(currentH, nextH, currentPercent));
				if (!lerping) {
					currentIteration++;
					//std::cout << "Iter" << std::endl;
					currentPercent = 0.0f;
					lerping = true;
				}
			}
			else {
				currentIteration = 0;
				currentEpisode++;
				//std::cout << "Episode: " << currentEpisode << std::endl;
			}
		}
		else {
			//std::cout << "Done" << std::endl;
		}
	}
}

void Game::render()
{
	SDL_RenderClear(m_renderer);
	SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);

	//std::cout << "Pre: " << *r << "," << *g << "," << *b << "," << *a << std::endl;
	//Do stuff
	//m_test.render(m_renderer);
	env.render(*m_renderer);
	agent->m_sprite.render(m_renderer);
	SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
	renderUI();
	SDL_RenderPresent(m_renderer);
}

void Game::run()
{
	float timeSinceLastUpdate = 0.f;
	float timePerFrame = 1.f / 60.f; // 60 fps in ms
	float currentTime = SDL_GetTicks() / 1000.0f;
	while (!m_quit)
	{
		float frameStart = SDL_GetTicks() / 1000.0f;
		float frameTime = frameStart - currentTime;
		currentTime = frameStart;
		ImGui_ImplSDL2_NewFrame(m_window);
		ImGui::NewFrame();

		processEvents(); // as many as possible
		timeSinceLastUpdate += frameTime;
		while (timeSinceLastUpdate > timePerFrame)
		{
			timeSinceLastUpdate -= timePerFrame;
			processEvents(); // at least 60 fps
			update(timePerFrame); //60 fps
		}
		render(); // as many as possible
	}
}

void Game::processEvents()
{
	//std::cout << "PE" << std::endl;
	while (SDL_PollEvent(&m_event)) {
		ImGui_ImplSDL2_ProcessEvent(&m_event);
		if (m_event.type == SDL_QUIT)
			m_quit = true;
		if (m_event.type == SDL_WINDOWEVENT && m_event.window.event == SDL_WINDOWEVENT_CLOSE && m_event.window.windowID == SDL_GetWindowID(m_window))
			m_quit = true;
	}
}

void Game::saveEpisode()
{
}

void Game::runAlgorithm()
{
	if (!m_algoStarted) {
		if (ImGui::Begin("Configuration")) {
			if (ImGui::GetID("Simulation")) {
				static bool disabled = false;
				ImGui::Selectable("Simulation", disabled);
			}
			ImGui::End();
		}
		resetAlgorithm();
		/*std::ofstream ofs;
		ofs.open("Logs/Log.txt", std::ofstream::out | std::ofstream::trunc);
		ofs.close();
		ofs.open("Logs/Log.txt", std::ofstream::app);
		std::vector<PlottableData> plotPoints;*/
		m_algoStarted = true;
		for (int i = 0; i < numEpisodes; ++i) {
			std::cout << "Episode: " << i << std::endl;
			std::cout << "=================================================" << std::endl;
			int iter_episode = 0;
			float reward_episode = 0;
			auto state = env.reset();
			std::vector<EpisodeVals> episodeData;
			while (true) {
				//std::cout << "Iteration : " << index << std::endl;
				//std::cout << "%%%%%%%%%%%%%%" << std::endl;
				int action;
				if (current_item == "Q Learning") {
					action = agent->getAction(env);
				}
				else {
					action = agent->getActionRBMBased(env);
				}
				
				auto state_vals = env.step(action);
				auto state_next = std::get<0>(state_vals);
				//std::cout << "S:" << state_next.first << "," << state_next.second << std::endl;
				auto reward = std::get<1>(state_vals);
				bool done = std::get<2>(state_vals);

				agent->train(std::make_tuple(state, action, state_next, reward, done));

				iter_episode++;
				reward_episode += reward;
				//std::cout << "R:" << reward_episode << std::endl;
				EpisodeVals vals;
				vals.action = action;
				vals.state = state;
				vals.nextState = state_next;
				vals.reward = reward;
				episodeData.push_back(vals);
				state = state_next;
				if (iter_episode >= maxIterations)
					done = true;
				if (done) {
					env.createHeatmapVals();
					break;
				}
			}
			agent->epsilon = std::fmax(agent->epsilon * agent->epsilonDecay, 0.01);
			/*ofs << "Episode: " << std::to_string(i) << "\n";
			for (auto & episodeVals : episodeData) {
				ofs << std::to_string(episodeVals.action) << " " << std::to_string(episodeVals.nextState.first) << "," << std::to_string(episodeVals.nextState.second) << " " << std::to_string(episodeVals.reward) << "\n";
			}
			*/
			std::cout << "Episode: " << i << " /" << numEpisodes << " Eps: " << agent->epsilon << " iter: " << iter_episode << " Rew: " << reward_episode << std::endl;

			PlottableData p;
			p.episodeNumber = i;
			p.rewardvalue = reward_episode;
			plotPoints.push_back(p);
			m_episodes.push_back(episodeData);
		}
		//ofs.close();
		agent->displayGreedyPolicy();
		m_algoStarted = false;
		m_algoFinished = true;

		// Log plot points for gnu plot
		/*std::ofstream plots;
		plots.open("Logs/Plots.txt", std::ofstream::out | std::ofstream::trunc);
		plots.close();
		plots.open("Logs/Plots.txt", std::ofstream::app);
		for (auto & point : plotPoints) {
			plots << point.episodeNumber << "	" << point.rewardvalue << "\n";
		}
		plots.close();*/
	}
}

void Game::startSimulation()
{
	m_simulationStarted = true;
	m_simulationFinished = false;
	currentEpisode = 0;
	currentIteration = 0;
	currentPercent = 0;
}

void Game::resetSimulation()
{
	m_simulationStarted = false;
	m_simulationFinished = false;
	currentEpisode = 0;
	currentIteration = 0;
	currentPercent = 0;
}

void Game::resetAlgorithm()
{
	m_episodes.clear();
	m_algoStarted = false;
	m_algoFinished = false;
	env.reset();
	agent->reset();
}

void Game::renderUI()
{
	ImGui::ShowDemoWindow();
	bool open = true;
	ImGui::SetNextWindowPos(ImVec2(1, (m_windowHeight / 2) + 1));
	ImGui::SetNextWindowSize(ImVec2(((m_windowWidth / 5) * 4) - 1, (m_windowHeight / 2) - 1));
	if (ImGui::Begin("Configuration", &open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
		if (ImGui::Button("Simulation")) {
			runAlgorithm();
			startSimulation();
		}
		ImGui::InputInt("Num Episodes: ", &numEpisodes, 1, 100, ImGuiWindowFlags_NoMove);
		ImGui::InputInt("Num Iterations: ", &maxIterations, 1, 100, ImGuiWindowFlags_NoMove);
		ImGui::InputFloat("Lerp Percent", &lerpPercent, 0.01f, 0.1f, 3, ImGuiWindowFlags_NoMove);
		if (!plotPoints.empty()) {
			int stride = sizeof(struct PlottableData);
			std::string temp = "Average Reward Value";
			ImGui::PushItemWidth(800);
			ImGui::PlotLines(
				temp.c_str(), &plotPoints.at(0).rewardvalue, plotPoints.size(), 0, NULL, 0, FLT_MAX, ImVec2(0, ImGui::GetWindowHeight() / 4), stride
			);
		}
		// Animate a simple progress bar
		static float progress = 0.0f, progress_dir = 1.0f;
		//std::cout << currentEpisode << std::endl;
		progress = currentEpisode / (float)numEpisodes;
		char buf[32];
		sprintf_s(buf, "%d/%d", (int)(progress * numEpisodes), numEpisodes);
		ImGui::ProgressBar(progress, ImVec2(0.f, 0.f), buf);
		ImGui::End();
	}
	if (ImGui::Begin("Algorithms")) {
		if (ImGui::BeginCombo("Algorithm", current_item, ImGuiComboFlags_HeightRegular)) {
			for (int n = 0; n < IM_ARRAYSIZE(items); n++)
			{
				bool is_selected = (current_item == items[n]);
				if (ImGui::Selectable(items[n], is_selected))
					current_item = items[n];
				if (is_selected)
					ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
			}
			ImGui::EndCombo();
		}
		ImGui::End();
	}
	ImGui::EndFrame();
	ImGui::Render();
	ImGuiSDL::Render(ImGui::GetDrawData());
}

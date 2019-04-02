#include "Game.h"
#include <iostream>
#include <fstream>
#include <SDL_image.h>
#include "MathUtils.h"

Game::Game()
{
	m_window = NULL;
	m_renderer = NULL;
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
	}
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
	//agent = new Agent(env, m_renderer);

	m_agents.clear();
	m_agentDone.clear();
	m_agentLerping.clear();
	m_agentIterations.clear();
	m_lerpPercentages.clear();
	for (int i = 0; i < m_numAgents; ++i) {
		m_agents.push_back(new Agent(env, m_renderer));
		m_lerpPercentages.push_back(0);
		m_agentDone.push_back(false);
		m_agentLerping.push_back(false);
		m_agentIterations.push_back(0);
	}
	current_item = items[2];
	m_numAgents = 2;

	const unsigned int numInput = env.stateDim.first * env.stateDim.second;
	const unsigned int numOutput = 4;
	const unsigned int numLayers = 3;
	const unsigned int numHidden = 4;
	const float desiredError = 0.01f;
	const unsigned int max_epochs = 500000;
	const unsigned int epochs_between_reports = 1000;

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
		if (currentEpisode < m_episodeData.size()) {
			for (int i = 0; i < m_agents.size(); ++i) {
				if (!m_agentDone.at(i)) {
					auto & agent = m_agents.at(i);
					auto & episode = m_episodeData.at(currentEpisode).at(i);
					m_agentLerping.at(i) = true;
					if (m_agentIterations.at(i) < episode.size()) {
						if (m_lerpPercentages.at(i) < lerpMax)
							m_lerpPercentages.at(i) += lerpPercent;
						else {
							m_lerpPercentages.at(i) = 1.0f;
							m_agentLerping.at(i) = false;
						}
						auto & data = episode.at(m_agentIterations.at(i));
						auto & nextState = data.nextState;
						auto & state = data.state;
						int w = env.cellW;
						int h = env.cellH;
						int currentW = w * state.second;
						int currentH = h * state.first;
						int nextW = w * nextState.second;
						int nextH = h * nextState.first;
						agent->m_sprite.setPosition(mu::lerp(currentW, nextW, m_lerpPercentages.at(i)), mu::lerp(currentH, nextH, m_lerpPercentages.at(i)));
						if (!m_agentLerping.at(i)) {
							m_agentIterations.at(i) += 1;
							//std::cout << "Iter" << std::endl;
							m_lerpPercentages.at(i) = 0.0f;
							m_agentLerping.at(i) = true;
							// TODO set agent orientation here
						}
					}
					else {
						m_agentDone.at(i) = true;
						if (!(std::find(m_agentDone.begin(), m_agentDone.end(), false) != m_agentDone.end())) {
							for (int j = 0; j < m_agents.size(); ++j) {
								m_agentIterations.at(j) = 0;
								m_lerpPercentages.at(j) = 0;
								m_agentLerping.at(j) = false;
								m_agentDone.at(j) = false;
							}
							currentEpisode++;
							if (currentEpisode == m_episodeData.size()) {
								break;
							}
						}
					}
				}
			}
		}
	}
}

void Game::multiThreadedUpdate(float deltaTime)
{

}

void Game::render()
{
	SDL_RenderClear(m_renderer);
	SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);

	//std::cout << "Pre: " << *r << "," << *g << "," << *b << "," << *a << std::endl;
	//Do stuff
	//m_test.render(m_renderer);
	env.render(*m_renderer);
	for (auto agent : m_agents) {
		agent->render(*m_renderer);
	}
	SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
	renderUI();
	SDL_RenderPresent(m_renderer);
}

void Game::run()
{
	float timeSinceLastUpdate = 0.f;
	float timePerFrame = 1.f / 1000.f; // 60 fps in ms
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
		switch (m_event.type)
		{
		case SDL_MOUSEBUTTONDOWN: {
			int x = m_event.button.x;
			int y = m_event.button.y;
			if (x > env.gridPosX && x < env.gridPosX + (env.cellW * env.stateDim.second)
				&& y > env.gridPosY && y < env.gridPosY + (env.cellH * env.stateDim.first)) {
				if (m_event.button.button == SDL_BUTTON_LEFT) {
					env.addObstacle(y / env.cellH, x / env.cellW);
				}
				else if (m_event.button.button == SDL_BUTTON_RIGHT) {
					env.addGoal(y / env.cellH, x / env.cellW);
				}
			}
			break;
		}
		default:
			break;
		}
	}
}

void Game::saveEpisode()
{
}

void Game::runAlgorithm()
{
	if (!m_algoStarted) {
		float currentTime = SDL_GetTicks() / 1000.0f;
		float timeDif = 0;
		m_agents.clear();
		m_agentDone.clear();
		m_agentLerping.clear();
		m_agentIterations.clear();
		m_lerpPercentages.clear();
		for (int i = 0; i < m_numAgents; ++i) {
			m_agents.push_back(new Agent(env, m_renderer));
			m_lerpPercentages.push_back(0);
			m_agentDone.push_back(false);
			m_agentLerping.push_back(false);
			m_agentIterations.push_back(0);
		}

		plotPoints.clear();

		resetAlgorithm();
		m_algoStarted = true;
		m_episodeData.clear();

		for (int i = 0; i < numEpisodes; ++i) {
			std::cout << "=================================================" << std::endl;
			std::vector<std::vector<EpisodeVals>> episodeData;
			episodeData.resize(m_agents.size());
			std::vector<AgentTrainingValues> agentVals;
			for (int i = 0; i < m_agents.size(); ++i) {
				auto & agent = m_agents.at(i);
				agent->m_done = false;
				auto states = env.getSpawnablePoint();
				std::pair<int, int> state = states.at(std::rand() % states.size());
				agent->m_previousState = state;
				agent->m_currentState = state;
				agentVals.push_back(AgentTrainingValues(env));
			}
			if (m_multiThreaded) {
				m_threads.clear();
				m_threads.resize(m_numAgents);
				for (int i = 0; i < m_agents.size(); ++i) {
					m_threads.push_back(agentSim(m_agents.at(i), &agentVals, i));
				}
			}
			while (true) {
				if (!m_multiThreaded) {
					/*std::cout << "Iteration : " << index << std::endl;
					std::cout << "%%%%%%%%%%%%%%" << std::endl;*/
					int currentAgent = 0;
					for (auto agent : m_agents) {
						if (!agent->m_done) {
							int action;
							if (current_item == "Q Learning")
								action = agent->getAction(env);
							else if (current_item == "RBM")
								action = agent->getActionRBMBased(env);
							else if (current_item == "MultiRBM")
								action = agent->getMultiAgentActionRBM(env, agentVals.at(currentAgent).iter_episode, maxIterations);

							auto state_vals = env.step(action, agent->m_currentState);
							auto state_next = std::get<0>(state_vals);
							auto reward = std::get<1>(state_vals);
							if (reward == -1) {
								agentVals.at(currentAgent).m_numCollisions++;
							}
							bool done = std::get<2>(state_vals);
							agent->m_previousState = agent->m_currentState;
							agent->train(std::make_tuple(agent->m_currentState, action, state_next, reward, done));
							agent->setOrientation(action);
							agent->m_currentState = state_next;
							env.setAgentFlags(agent->m_previousState, agent->m_currentState);

							EpisodeVals vals;
							vals.action = action;
							vals.state = agent->m_previousState;
							vals.nextState = state_next;
							episodeData.at(currentAgent).push_back(vals);
							
							agentVals.at(currentAgent).iter_episode += 1;
							agentVals.at(currentAgent).reward_episode += reward;
							agentVals.at(currentAgent).state = state_next;
							if (agentVals.at(currentAgent).iter_episode >= maxIterations || done)
								agent->m_done = true;
							/*std::cout << "================" << std::endl;
							for (auto & row : env.m_tileFlags) {
								for (auto & flag : row) {
									std::cout << (flag & QLCContainsAgent) << " ";
								}
								std::cout << std::endl;
							}*/
						}
						currentAgent++;
					}
				}

				// Only finish when all agents are finished
				auto pred = [](const Agent *a) {
					return !a->m_done;
				};
				if (!(std::find_if(m_agents.begin(), m_agents.end(), pred) != m_agents.end()))
					break;
			}
			for (auto agent : m_agents) {
				agent->epsilon = std::fmax(agent->epsilon * agent->epsilonDecay, 0.01);
			}

			int currentAgent = 0;
			for (auto agent : m_agents) {
				std::cout << "Episode: " << i << " /" << numEpisodes << " Eps: " << agent->epsilon << " iter: " << agentVals.at(currentAgent).iter_episode << " Rew: " << agentVals.at(currentAgent).reward_episode << " Num Cols: " << agentVals.at(currentAgent).m_numCollisions << std::endl;
				currentAgent++;
			}
			if (!m_multiThreaded) {
				//for (int i = 0; i < m_agents.size(); ++i) {
				//PlottableData p;
				//p.episodeNumber = i;
				//p.rewardvalue = agentVals.at(i).reward_episode;
				//plotPoints.at(i).push_back(p);
				//}
				m_episodeData.push_back(episodeData);

			}
			for (auto & thread : m_threads) {
				if (thread.joinable())
					thread.join();
			}
		}

		// Display the final policy
		for (auto agent : m_agents) {
			std::cout << "Agent: " << std::endl;
			agent->displayGreedyPolicy();
		}
		env.createHeatmapVals();
		m_algoStarted = false;
		m_algoFinished = true;
		timeDif = (SDL_GetTicks() / 1000) - currentTime;
		std::cout << "TD : " << timeDif << std::endl;
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
	m_episodeData.clear();
	m_algoStarted = false;
	m_algoFinished = false;
	env.reset();
	for (auto agent : m_agents) {
		agent->reset();
	}
}

void Game::renderUI()
{
	bool open = true;
	ImGui::SetNextWindowPos(ImVec2(1, m_windowHeight / 2));
	ImGui::SetNextWindowSize(ImVec2(((m_windowWidth / 5) * 4), (m_windowHeight / 2)));
	if (ImGui::Begin("Configuration", &open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
		ImGui::DragInt("Num Agents", &m_numAgents, 1, 1, 100);
		ImGui::DragInt("Xsize", &env.xSize, 1, 1, 100);
		ImGui::DragInt("Ysize", &env.ySize, 1, 1, 100);
		if (ImGui::Button("Generate Env")) {
			env.init(env.xSize, env.ySize);
			for (auto & agent : m_agents) {
				agent->m_sprite.setBounds(env.cellW, env.cellH);
			}
		}
		ImGui::InputInt("Num Episodes: ", &numEpisodes, 1, 100, ImGuiWindowFlags_NoMove);
		ImGui::InputInt("Num Iterations: ", &maxIterations, 1, 100, ImGuiWindowFlags_NoMove);
		ImGui::InputFloat("Lerp Percent", &lerpPercent, 0.01f, 0.1f, 3, ImGuiWindowFlags_NoMove);
		ImGui::Checkbox("Multithreaded", &m_multiThreaded);
		if (ImGui::Button("Simulation")) {
			for (auto & agent : m_agents) {
				agent->m_sprite.setBounds(env.cellW, env.cellH);
			}
			runAlgorithm();
			startSimulation();
		}
		if (!plotPoints.empty()) {
			int stride = sizeof(struct PlottableData);
			std::string temp = "Average Reward Value";
			ImGui::PushItemWidth(800);
			for (auto & plotPoints : plotPoints) {
				ImGui::PlotLines(
					temp.c_str(), &plotPoints.at(0).rewardvalue, plotPoints.size(), 0, NULL, 0, FLT_MAX, ImVec2(0, ImGui::GetWindowHeight() / 4), stride
				);
			}
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

void Game::runRuleBased()
{
}

void Game::runQLearning()
{
}

void Game::runGeneralQ()
{
}

void Game::runMARLQ()
{
}

std::thread Game::agentSim(Agent * agent, std::vector<AgentTrainingValues> * agentVals, int currentAgent)
{
	return std::thread([=] {
		while (!agent->m_done) {
			int action;
			if (current_item == "Q Learning") {
				action = agent->getAction(env);
			}
			else {
				action = agent->getActionRBMBased(env);
			}
			auto state_vals = env.step(action, agent->m_currentState);
			auto state_next = std::get<0>(state_vals);
			auto reward = std::get<1>(state_vals);
			bool done = std::get<2>(state_vals);
			agent->m_previousState = agent->m_currentState;
			agent->train(std::make_tuple(agent->m_currentState, action, state_next, reward, done));

			/*	EpisodeVals vals;
			vals.action = action;
			vals.state = agent->m_currentState;
			vals.nextState = state_next;
			episodeData.at(currentAgent).push_back(vals);*/
			agent->setOrientation(action);
			agent->m_currentState = state_next;

			agentVals->at(currentAgent).iter_episode += 1;
			agentVals->at(currentAgent).reward_episode += reward;
			agentVals->at(currentAgent).state = state_next;
			if (agentVals->at(currentAgent).iter_episode >= maxIterations || done)
				agent->m_done = true;
		}
	});
}

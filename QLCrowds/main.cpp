#include <iostream>

#include "Environment.h"
#include "Agent.h"

void main() {
	Environment env;
	Agent agent(env);

	int numEpisodes = 500;
	for (int i = 0; i < numEpisodes; ++i) {
		std::cout << "Episode: " << i << std::endl;
		std::cout << "=================================================" << std::endl;
		int iter_episode = 0;
		float reward_episode = 0;
		auto state = env.reset();
		while (true) {
			//std::cout << "Iteration : " << index << std::endl;
			//std::cout << "%%%%%%%%%%%%%%" << std::endl;
			auto action = agent.getAction(env);
			auto state_vals = env.step(action);
			auto state_next = std::get<0>(state_vals);
			//std::cout << "S:" << state_next.first << "," << state_next.second << std::endl;
			auto reward = std::get<1>(state_vals);
			auto done = std::get<2>(state_vals);

			agent.train(std::make_tuple(state, action, state_next, reward, done));

			iter_episode++;
			reward_episode += reward;
			//std::cout << "R:" << reward_episode << std::endl;

			if (done) {
				break;
			}
			state = state_next;
		}
		agent.epsilon = std::fmax(agent.epsilon * agent.epsilonDecay, 0.01);

		std::cout << "Episode: " << i << " /" << numEpisodes << " Eps: " << agent.epsilon << " iter: " << iter_episode << " Rew: " << reward_episode << std::endl;
	}
	agent.displayGreedyPolicy();
	system("pause");
}
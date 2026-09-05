#pragma once

#include<vector>
#include<string>
#include<memory>
#include"vec2.hpp"

class AgentComm {
private:
  int num_agents;
  std::unique_ptr<unsigned[]> signals;
public:
  AgentComm(int num_agents)
    : num_agents(num_agents)
  {
    signals = std::make_unique<unsigned[]>(num_agents);
  }
  int GetNumAgents() const {
    return num_agents;
  }
  unsigned GetSignal(int index) const {
    return signals[index];
  }
  void SetSignal(int index, unsigned signal) {
    signals[index] = signal;
  }
};

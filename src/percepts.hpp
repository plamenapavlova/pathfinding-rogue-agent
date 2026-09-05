#pragma once

#include<vector>
#include<string>

enum class AgentType {
    AGENT,
    NONE
};

struct Percepts {
    std::vector<std::string> current;
    std::vector<std::string> forward;
    std::vector<std::string> backward;
    std::vector<std::string> left;
    std::vector<std::string> right;
    int detector;
};

struct Symbols {
  std::vector<std::string> teleporters;
  std::string wall;
  std::string open;
  std::string disarmed_mine;
  std::string exploded_mine;
};

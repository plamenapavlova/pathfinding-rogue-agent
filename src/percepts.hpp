#pragma once

#include<vector>
#include<string>
#include<utility>

#include"vec2.hpp"

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
  std::vector<Vec2> others;
};

struct Symbols {
  std::vector<std::string> teleporters;
  std::string wall;
  std::string open;
  std::string disarmed_mine;
  std::string exploded_mine;
  std::string treasure;
};

struct Costs {
  int round_cost;
  int move_cost;
  int turn_cost;
  int tele_cost;
  int crash_cost;
  int notele_cost;
  int notrap_cost;
  int death_cost;
  int disarm_cost;
  int inactive_cost;
  int treasure_cost;
  int notreasure_cost;
};

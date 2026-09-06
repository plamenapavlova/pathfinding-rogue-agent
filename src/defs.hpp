#pragma once

#include<vector>
#include<string>
#include<array>
#include"vec2.hpp"
#include"comm.hpp"

inline const float CELL_SIZE = 24.0f;

using wmap = std::vector<std::vector<std::string>>;

struct WorldConfig {
  unsigned num_agents;
  unsigned agent_speed;
  std::array<unsigned, 4> agent_sight;
  std::string wall;
  std::string open;
  std::string mine;
  std::string disarmed_mine;
  std::string exploded_mine;
  std::vector<std::string> tele_symbols;
  std::vector<unsigned> tele_dests;
  std::string agent_start;
  std::string treasure;
  std::vector<unsigned> wall_color;
  std::vector<unsigned> open_color;
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
  double slip_chance;
};

struct AgentSight {
  unsigned forward;
  unsigned behind;
  unsigned left;
  unsigned right;
};



inline const Vec2 NORTH(0, -1);
inline const Vec2 SOUTH(0, 1);
inline const Vec2 EAST(1, 0);
inline const Vec2 WEST(-1, 0);

inline const std::vector<std::string> VALID_CMDS = {
  "F", "B",
  "L", "R",
  "U", "T",
  "D"
};

enum class Result {
  NONE,
  TELEPORT
};




#pragma once

#include<algorithm>
#include<string>
#include<random>
#include<map>
#include<cstdlib>
#include<iostream>
#include<fstream>
#include"percepts.hpp"
#include"comm.hpp"

#include <set>
#include <utility>

class AI {
protected:
  // Necessary, do not delete.
  unsigned id;
  unsigned agent_speed;
  std::mt19937_64* rng;
  Symbols symbols;
  Costs costs;

  //new
  Vec2 my_location= Vec2(0, 0);//starting location
  Vec2 my_heading = Vec2(0, -1);//starting heading
  Vec2 internal_north = Vec2(0, 1);
  std::set<std::pair<int, int>> safe_cells;//stores the computed safe cells including walls
  std::map<std::pair<int, int>, std::string> known_map;//what the agent has seen


public:
  AI();
  AI(
     unsigned id, 
     unsigned agent_speed,
     std::mt19937_64* rng,
     Symbols symbols,
     Costs costs);
  void PrintPercepts(const Percepts & percepts);
  std::vector<std::string> Run(
			       Percepts & percepts,
			       AgentComm * comms);

  //new
  bool CheckSafety(Vec2 loc);
  void MarkSafe(Vec2 loc);
  void SafeZone(Vec2 loc, int trap_dist);
  void UpdateLocation(std::string cmd);
  void UpdateMap(Percepts & percepts);
  //teleporter mapping

};




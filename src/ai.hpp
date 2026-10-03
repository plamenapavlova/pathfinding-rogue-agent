#pragma once

#include<algorithm>
#include<string>
#include<random>
#include<map>
#include<cstdlib>
#include<iostream>
#include<fstream>
#include<optional>
#include<deque>
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
  Vec2 internal_north = Vec2(0, -1);//agent's north direction
  int current_turn = 0;
  std::optional<Vec2> pending_disarm_cell;//the cell that is being disarmed
  std::optional<Vec2> current_goal; // location of the treasure
  struct AdjacentCells {
      Vec2 forward_cell, right_cell, left_cell, backward_cell;
  };
  struct SafeDirections {
      bool forward_safe, right_safe, left_safe, backward_safe;
  };
  struct Directions {
      Vec2 forward, right, left, backward;
  };
  std::deque<std::pair<int, int>> last_visited_cells; // the last 9 cells the agent has psyhically occupied
  std::deque<std::string> pending_commands; // the commands that the agent has committed to
  std::set<std::pair<int, int>> dead_ends;//stores the ends of the maze
  std::set<std::pair<int, int>> safe_cells;//stores the computed safe cells including walls
  std::map<std::pair<int, int>, std::string> known_map;//what the agent has seen
  std::set<std::pair<int, int>> visited_cells;//which cells has the agent physically occupied
  
  int max_turn;

public:
  AI();
  AI(
     unsigned id, 
     unsigned agent_speed,
     std::mt19937_64* rng,
     Symbols symbols,
     Costs costs,
     int max_turn);
  void PrintPercepts(const Percepts & percepts);
  std::vector<std::string> Run(
			       Percepts & percepts,
			       AgentComm * comms);

  //new
  bool CheckSafety(Vec2 loc);
  void MarkSafe(Vec2 loc);
  void SafeZone(Vec2 loc, int trap_dist);
  void UpdateLocation(std::string cmd);
  void UpdateMap(const Percepts & percepts);
  //teleporter mapping
  std::string DecideAction(const Percepts& percepts);
  std::optional<Vec2> FindNearestObject(const std::string& object_symbol);
  std::string MoveTowardTarget(Vec2 target, const Percepts& percepts);
  int CalculateUnexploredCells(Vec2 direction);
  std::optional<Vec2> DecideSeekTeleporter(const Percepts& percepts);
  AdjacentCells GetAdjacentCells();
  SafeDirections GetSafeDirections(const Percepts& percepts, const AdjacentCells& adj_cells);
  Directions GetDirections();

  std::optional<std::string> TrapHunting(const Percepts& percepts, const AdjacentCells& adj_cells);
  std::optional<std::string> TreasureHunting(const Percepts& percepts);
  std::optional<std::string> LoopDetection(const SafeDirections& safe);
  std::optional<std::string> UseTeleporter(const Percepts& percepts);
  void UpdateDeadEnds(const AdjacentCells& adj_cells, const Percepts& percepts);
  bool WallOrDead(Vec2 cell);
  std::optional<std::string> DecideExploration(const SafeDirections& safe);
  std::string FallBackDeadEnd();
  void MarkMapSafe();
};




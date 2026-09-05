#pragma once

#include<cstdio>
#include<fstream>
#include<iostream>
#include<vector>
#include<exception>
#include<algorithm>
#include"json.hpp"
#include"defs.hpp"
#include"vec2.hpp"
#include"percepts.hpp"
#include"defs.hpp"

#include<iostream>

struct Teleporter {
  unsigned id;
  unsigned dest;
  std::string symbol;
  Vec2 location;
};


struct World {


  WorldConfig config;
  wmap map;
  std::vector<Vec2> agent_starts;
  std::vector<Teleporter> teleporters;
  std::vector<Vec2> mine_locations;
  unsigned num_mines;
  unsigned num_mines_remaining;
  Symbols symbols;

  World();
  bool LoadConfig(std::string & filename);
  bool LoadMap(std::string & filename);

  std::string GetCell(Vec2 location);
  void SetCell(Vec2 location, std::string s);
  std::vector<std::string> GetCells(
				    Vec2 loc, Vec2 heading, int dist, 
				    std::vector<Vec2> & seen_cells
				    );
  bool CanMoveIntoCell(Vec2 location);
  Result UseCell(Vec2 location, Vec2 & result_location);
  unsigned GetNumMines() const;
  unsigned GetNumMinesRemaining() const;
  void ChangeNumMinesRemaining(unsigned amt);
  bool HasHitMine(const Vec2 & loc);
  bool DisarmMine(const Vec2 & loc);
  int ManhattanDistanceToNearestMine(const Vec2 & loc);
  Symbols GetSymbols() const;
};

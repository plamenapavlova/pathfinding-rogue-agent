#include"ai.hpp"

/***************************************************************
AI CLASS DEFINITION
*/
AI::AI() {}
AI::AI(
    unsigned id, 
    unsigned agent_speed,
    std::mt19937_64 * rng,
    Symbols symbols,
    Costs costs
)
  : id(id), agent_speed(agent_speed), rng(rng),
    symbols(symbols), costs(costs)
{}

void AI::PrintPercepts(const Percepts & percepts) {
  std::cout << "DISTANCE: " << percepts.detector << std::endl;
  std::cout << "CURRENT:  " << percepts.current[0] << std::endl;
  std::cout << "FORWARD:  ";
  for(std::vector<std::string>::const_iterator it = percepts.forward.begin();
      it != percepts.forward.end(); it++) std::cout << *it << " ";
  std::cout << std::endl;
  std::cout << "LEFT:     ";
  for(std::vector<std::string>::const_iterator it = percepts.left.begin();
      it != percepts.left.end(); it++) std::cout << *it << " ";
  std::cout << std::endl;
  std::cout << "BACKWARD: ";
  for(std::vector<std::string>::const_iterator it = percepts.backward.begin();
      it != percepts.backward.end(); it++) std::cout << *it << " ";
  std::cout << std::endl;
  std::cout << "RIGHT:    ";
  for(std::vector<std::string>::const_iterator it = percepts.right.begin();
      it != percepts.right.end(); it++) std::cout << *it << " ";
  std::cout << std::endl;
  std::cout << "Others:\n";
  for(size_t i = 0; i < percepts.others.size(); i++) {
    std::cout << "   " << i << ": " << percepts.others[i].to_string() << std::endl;
  }
}
//new

bool AI::CheckSafety(Vec2 loc) {
    if (safe_cells.find({ loc.x, loc.y }) == safe_cells.end()) return false; return true;
}

void AI::MarkSafe(Vec2 loc) {
    safe_cells.insert({ loc.x, loc.y });
}

void AI::SafeZone(Vec2 loc, int trap_dist) {
    if (trap_dist <= 0) return;

    for (int end_x = loc.x - (trap_dist - 1); end_x <= loc.x + trap_dist - 1; end_x++) {
		int remaining = trap_dist - 1 - std::abs(end_x - loc.x);
        for (int end_y = loc.y - remaining; end_y <= loc.y+remaining; end_y++) {
            MarkSafe(Vec2(end_x, end_y));
        }
    }
}
void AI::UpdateLocation(std::string cmd) {
    if (cmd == "F") {
        my_location = my_location + my_heading;
    }
    else if (cmd == "B") {
        my_location = my_location - my_heading;
    }
    else if (cmd == "L") {
        my_heading = Vec2(my_heading.y, -my_heading.x);
    }
    else if (cmd == "R") {
        my_heading = Vec2(-my_heading.y, my_heading.x);
    }
    else if (cmd == "U" || cmd == "D" || cmd == "T") {
    }
    else {
        std::cerr<<"UpdateLocation: Unexpected command"<<cmd<< "'\n";
    }
}

void AI::UpdateMap(Percepts& percepts) {
    if (!percepts.current.empty()) {
        known_map[{my_location.x, my_location.y}] = percepts.current[0];
    }
    Vec2 fwd = my_heading;
    for (size_t i = 0; i < percepts.forward.size(); i++) {
        Vec2 cell_loc = my_location + (int)(i+1) * fwd;
        known_map[{cell_loc.x, cell_loc.y}] = percepts.forward[i];
    }
    Vec2 back = Vec2( - my_heading.x, -my_heading.y);
    for (size_t i = 0; i < percepts.backward.size(); i++) {
        Vec2 cell_loc = my_location + int(i + 1) * back;
        known_map[{cell_loc.x, cell_loc.y}] = percepts.backward[i];
    }
    Vec2 left = Vec2(my_heading.y, -my_heading.x);
    for (size_t i = 0; i < percepts.left.size(); i++) {
        Vec2 cell_loc = my_location + (int)(i + 1) * left;
        known_map[{cell_loc.x, cell_loc.y}] = percepts.left[i];
    }
    Vec2 right = Vec2(-my_heading.y, my_heading.x);
    for (size_t i = 0; i < percepts.right.size(); i++) {
        Vec2 cell_loc = my_location + int(i + 1) * right;
        known_map[{cell_loc.x, cell_loc.y}] = percepts.right[i];
    }
}

std::vector<std::string> AI::Run(
    Percepts & percepts,
    AgentComm * comms
) {
  std::cout << "------------------------------------------------\n";
  std::cout << "AGENT ID: " << id << std::endl;
  PrintPercepts(percepts);
  std::vector<std::string> cmds {"R", "B", "L", "F", "U", "D"};

  
  std::shuffle(cmds.begin(), cmds.end(), *rng);
  std::cout << "CMD:      " << cmds[0] << std::endl;
  

  int trap_dist = percepts.detector;
  //if (trap_dist == 1){
	//  return { "D" };
  //}
  SafeZone(my_location,trap_dist);

  std::cout << "Safe cells: ";
  for (const auto& cell : safe_cells) {
      std::cout << "(" << cell.first << "," << cell.second << ") ";
  }
  std::cout << std::endl;
  UpdateLocation(cmds[0]);
  return { cmds[0] };

}




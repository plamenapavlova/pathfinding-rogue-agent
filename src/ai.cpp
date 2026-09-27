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
        visited_cells.insert({ my_location.x, my_location.y });
    }
    Vec2 fwd = my_heading;
    for (size_t i = 0; i < percepts.forward.size(); i++) {
        Vec2 cell_loc = my_location + fwd*(int)(i+1);
        known_map[{cell_loc.x, cell_loc.y}] = percepts.forward[i];
    }
    Vec2 back = Vec2( - my_heading.x, -my_heading.y);
    for (size_t i = 0; i < percepts.backward.size(); i++) {
        Vec2 cell_loc = my_location + back*int(i + 1);
        known_map[{cell_loc.x, cell_loc.y}] = percepts.backward[i];
    }
    Vec2 left = Vec2(my_heading.y, -my_heading.x);
    for (size_t i = 0; i < percepts.left.size(); i++) {
        Vec2 cell_loc = my_location + left* (int)(i + 1);
        known_map[{cell_loc.x, cell_loc.y}] = percepts.left[i];
    }
    Vec2 right = Vec2(-my_heading.y, my_heading.x);
    for (size_t i = 0; i < percepts.right.size(); i++) {
        Vec2 cell_loc = my_location + right * int(i + 1);
        known_map[{cell_loc.x, cell_loc.y}] = percepts.right[i];
    }
}
std::optional<Vec2> AI::FindNearestTreasure() {
    std::optional<Vec2> nearest;
	int shortest_dist = std::numeric_limits<int>::max();

    for (const auto& entry : known_map) {
        if (entry.second == symbols.treasure) {
            int dist = my_location.ManhattanDistance(Vec2(entry.first.first, entry.first.second));
            if (dist < shortest_dist) {
                shortest_dist = dist;
                nearest = Vec2(entry.first.first, entry.first.second);
            }
        }
    }
	return nearest;
}

std::string AI::MoveTowardTarget(Vec2 target, Percepts& percepts) {
    Vec2 diff = target - my_location;
    if (diff.x == 0 && diff.y == 0) {
        return "reached";
    }
    Vec2 direction;
    if (std::abs(diff.x) >= std::abs(diff.y)) {
        if (diff.x > 0 ) direction = Vec2(1, 0);
		else direction = Vec2(-1, 0);
    }
    else {
		if (diff.y > 0) direction = Vec2(0, 1);
		else direction = Vec2(0, -1);
    }

    if (my_heading == direction) {
		Vec2 next_cell = my_location + my_heading;
        if (CheckSafety(next_cell) && !percepts.forward.empty() && percepts.forward[0] != symbols.wall) {
            return "F";
        }
        else return "R"; //NEEDS SOME LOGIC
    }
    Vec2 right_of_current = Vec2(-my_heading.y, my_heading.x);
    if (right_of_current == direction) return "R";
    return "L";
}



std::string AI::DecideAction(Percepts& percepts) {
    Vec2 forward_cell = my_location + my_heading;
    //trap hunting mode
    if (percepts.detector == 1 && safe_cells.find({ forward_cell.x, forward_cell.y}) == safe_cells.end()) {
        pending_disarm_cell = forward_cell;
        return "D";
    }

    //treasure mode
    if (!current_goal.has_value()) {
        current_goal = FindNearestTreasure();
    }
    if (current_goal.has_value()) {
		std::string action = MoveTowardTarget(*current_goal, percepts);
        if (action == "reached") {
            known_map[{my_location.x, my_location.y}] = symbols.open;
			current_goal = std::nullopt;
            return "T";
        }
        return action;
    }
    
    
    //explore mode
    if (!pending_commands.empty()) {
		std::string next_cmd = pending_commands.front();
        pending_commands.pop_front();
        return next_cmd;
    }

    //dead end cells
    Vec2 backward_cell = my_location - my_heading;
    bool backward_is_dead_end = dead_ends.count({ backward_cell.x, backward_cell.y }) > 0;
    bool left_is_wall = !percepts.left.empty() && percepts.left[0] == symbols.wall;
    bool right_is_wall = !percepts.right.empty() && percepts.right[0] == symbols.wall;
    if (backward_is_dead_end && left_is_wall && right_is_wall) {
        dead_ends.insert({ my_location.x, my_location.y });
    }

    Vec2 right = Vec2(-my_heading.y, my_heading.x);
    Vec2 left = Vec2(my_heading.y, -my_heading.x);

    Vec2 right_cell = my_location + right;
	Vec2 left_cell = my_location + left;

    //check which directions are dead ends
    bool forward_is_dead_end = dead_ends.count({ forward_cell.x, forward_cell.y }) > 0;
    bool right_is_dead_end = dead_ends.count({ right_cell.x, right_cell.y }) > 0;
    bool left_is_dead_end = dead_ends.count({ left_cell.x, left_cell.y }) > 0;

    //-->safe options
    bool right_safe = CheckSafety(right_cell) && !percepts.right.empty() && percepts.right[0] !=symbols.wall && !right_is_dead_end;
	bool left_safe = CheckSafety(left_cell) && !percepts.left.empty() && percepts.left[0] != symbols.wall && !left_is_dead_end;
	bool forward_safe = CheckSafety(forward_cell) && !percepts.forward.empty() && percepts.forward[0] != symbols.wall && !forward_is_dead_end;


    
  


    
	//chech which directions have been visited
	bool forward_visited = visited_cells.count({ forward_cell.x, forward_cell.y }) > 0;
	bool right_visited = visited_cells.count({ right_cell.x, right_cell.y }) > 0;
	bool left_visited = visited_cells.count({ left_cell.x, left_cell.y }) > 0;

    //-->preferred options
    bool forward_pref = forward_safe && !forward_visited;
    bool right_pref = right_safe && !right_visited;
	bool left_pref = left_safe && !left_visited;

    std::vector<std::string> pref_directions;
    if (right_pref) pref_directions.push_back("R");
    if (left_pref) pref_directions.push_back("L");
    if (forward_pref) pref_directions.push_back("F");

	//randonmy choose from preferred options
    if (!pref_directions.empty()) {
        std::shuffle(pref_directions.begin(), pref_directions.end(), *rng);
        if (pref_directions[0] == "L" || pref_directions[0] == "R") pending_commands.push_back("F");
        return pref_directions[0];
    }

	std::vector<std::string> safe_directions;
	if (right_safe) safe_directions.push_back("R");
    if (left_safe) safe_directions.push_back("L");
	if (forward_safe) safe_directions.push_back("F");

	//fallback to safe options 
    if (!safe_directions.empty()) {
		std::shuffle(safe_directions.begin(), safe_directions.end(), *rng);
        if (safe_directions[0] == "L" || safe_directions[0] == "R") pending_commands.push_back("F");
		return safe_directions[0];
    }
    

	//if no safe directions, mark current cell as dead end and return
    dead_ends.insert({my_location.x, my_location.y});
	pending_commands.push_back("R");    
	pending_commands.push_back("F");
	return "R";

}

std::vector<std::string> AI::Run(Percepts & percepts,AgentComm * comms) {
  std::cout << std::unitbuf;
  std::cout << "------------------------------------------------\n";
  std::cout << "AGENT ID: " << id << std::endl;
  PrintPercepts(percepts);

  //std::vector<std::string> cmds {"R", "B", "L", "F", "U", "D"};
  //std::shuffle(cmds.begin(), cmds.end(), *rng);
  //std::cout << "CMD:      " << cmds[0] << std::endl;

  UpdateMap(percepts);
  std::cout << "Known map (" << known_map.size() << " cells):\n";
  for (const auto& entry : known_map) {
      std::cout << "  (" << entry.first.first << "," << entry.first.second
          << "): '" << entry.second << "'\n";
  }

  if (pending_disarm_cell.has_value()) {
      MarkSafe(pending_disarm_cell.value());
      pending_disarm_cell = std::nullopt;
  }





  int trap_dist = percepts.detector;
  SafeZone(my_location,trap_dist);
    
  std::cout << "Safe cells: ";
  for (const auto& cell : safe_cells) {
      std::cout << "(" << cell.first << "," << cell.second << ") ";
  }
  std::cout << std::endl;

  


  
  std::string action = DecideAction(percepts);
  std::cout << "Action: " << action << std::endl;
  UpdateLocation(action);
  return {action};

}




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
    Costs costs,
    int max_turn
)
  : id(id), agent_speed(agent_speed), rng(rng),
    symbols(symbols), costs(costs), max_turn(max_turn)
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

AI::AdjacentCells AI::GetAdjacentCells() {
    AdjacentCells adj_cells;

    Vec2 right = Vec2(-my_heading.y, my_heading.x);
    Vec2 left = Vec2(my_heading.y, -my_heading.x);

    adj_cells.forward_cell = my_location + my_heading;
	adj_cells.right_cell = my_location + right;
	adj_cells.left_cell = my_location + left;
	adj_cells.backward_cell = my_location - my_heading;

	return adj_cells;

}

AI::SafeDirections AI::GetSafeDirections(const Percepts& percepts, const AdjacentCells& adj_cells) {
    SafeDirections safe_dirs;


    bool forward_is_dead_end = dead_ends.count({ adj_cells.forward_cell.x, adj_cells.forward_cell.y }) > 0;
    bool right_is_dead_end = dead_ends.count({ adj_cells.right_cell.x, adj_cells.right_cell.y }) > 0;
    bool left_is_dead_end = dead_ends.count({ adj_cells.left_cell.x, adj_cells.left_cell.y }) > 0;
    bool backward_is_dead_end = dead_ends.count({ adj_cells.backward_cell.x, adj_cells.backward_cell.y }) > 0;

    //-->safe options
    safe_dirs.right_safe = CheckSafety(adj_cells.right_cell) && !percepts.right.empty() && percepts.right[0] != symbols.wall && !right_is_dead_end;
    safe_dirs.left_safe = CheckSafety(adj_cells.left_cell) && !percepts.left.empty() && percepts.left[0] != symbols.wall && !left_is_dead_end;
    safe_dirs.forward_safe = CheckSafety(adj_cells.forward_cell) && !percepts.forward.empty() && percepts.forward[0] != symbols.wall && !forward_is_dead_end;
    safe_dirs.backward_safe = CheckSafety(adj_cells.backward_cell) && !percepts.backward.empty() && percepts.backward[0] != symbols.wall && !backward_is_dead_end;

    return safe_dirs;
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

void AI::UpdateMap(const Percepts& percepts) {
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
std::optional<Vec2> AI::FindNearestObject(const std::string& object_symbol) {
    std::optional<Vec2> nearest;
	int shortest_dist = std::numeric_limits<int>::max();

    for (const auto& entry : known_map) {
        if (entry.second == object_symbol) {
            int dist = my_location.ManhattanDistance(Vec2(entry.first.first, entry.first.second));
            if (dist < shortest_dist) {
                shortest_dist = dist;
                nearest = Vec2(entry.first.first, entry.first.second);
            }
        }
    }
	return nearest;
}

std::string AI::MoveTowardTarget(Vec2 target, const Percepts& percepts) {
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

int AI::CalculateUnexploredCells(Vec2 direction) {
    static const int EXPLORE_DIST = 15;
    Vec2 perpendicular_direction = Vec2(direction.y, -direction.x);
    int unexplored_cells = 0;

    for (int i = 1; i <= EXPLORE_DIST; i++) {
        Vec2 center_cell = my_location + direction * i;

        //check for walls 
        auto it = known_map.find({center_cell.x, center_cell.y});
        if (it != known_map.end()) {
            if (it->second == symbols.wall) break;
        }

        Vec2 adj_left = center_cell - perpendicular_direction;
        Vec2 adj_right = center_cell + perpendicular_direction;
        for (const auto& cell : { center_cell, adj_left, adj_right }) {
            if (known_map.find({ cell.x, cell.y }) != known_map.end()) continue;
            unexplored_cells += 1;
        }
    }
    return unexplored_cells;
}

std::optional<Vec2> AI::DecideSeekTeleporter(const Percepts& percepts) {
    static const double WAIT_FRACTION = 0.15;
    static const double END_FRACTION = 0.80;
    static const double BUFFER_FRACTION = 0.3;//max turns to reach teleporter

    int remaining_turns = max_turn - current_turn;
    int tel_min_turns = int(max_turn * WAIT_FRACTION);
    int tel_max_turns = int(max_turn * END_FRACTION);
    int max_acc_dist = int(remaining_turns * BUFFER_FRACTION);

    std::optional<Vec2> nearest_known_tel = std::nullopt;
    int shortest_dist = std::numeric_limits<int>::max();

    for (const std::string& tel_symbol : symbols.teleporters) {
        std::optional<Vec2> candidate = FindNearestObject(tel_symbol);
        if (candidate.has_value()) {
            int distance = my_location.ManhattanDistance(candidate.value());
            if (distance < shortest_dist) {
                shortest_dist = distance;
                nearest_known_tel = candidate;
            }
        }
    }

    if (!nearest_known_tel.has_value()) return std::nullopt;
    int tel_dist = my_location.ManhattanDistance(nearest_known_tel.value());
    if (current_turn >= tel_min_turns && current_turn <= tel_max_turns && tel_dist <= max_acc_dist) {
        return nearest_known_tel;
    }
    return std::nullopt;

    
}

std::optional<std::string> AI::TrapHunting(const Percepts& percepts, const AdjacentCells& adj_cells) {

    if (percepts.detector == 1 && safe_cells.find({ adj_cells.forward_cell.x, adj_cells.forward_cell.y }) == safe_cells.end()) {
        pending_disarm_cell = adj_cells.forward_cell;
        return "D";
    }
    return std::nullopt;
}

std::optional<std::string> AI::TreasureHunting(const Percepts& percepts) {
    if (!current_goal.has_value()) {
        current_goal = FindNearestObject(symbols.treasure);
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
    return std::nullopt;
}
std::optional<std::string> AI::LoopDetection(const SafeDirections& safe) {
    static const int MAX_LAST_VISITED_CELLS = 9;
    last_visited_cells.push_back({ my_location.x, my_location.y });
    if (last_visited_cells.size() > MAX_LAST_VISITED_CELLS) {
        last_visited_cells.pop_front();
    }
    int repeated_visits = std::count(last_visited_cells.begin(), last_visited_cells.end(),
        std::make_pair(my_location.x, my_location.y));
    if (repeated_visits >= 3) {
        std::vector<std::string> candidates;
        if (safe.right_safe) candidates.push_back("R");
        if (safe.left_safe) candidates.push_back("L");
        if (safe.forward_safe) candidates.push_back("F");
        if (!candidates.empty()) {
            std::shuffle(candidates.begin(), candidates.end(), *rng);
            std::string winner = candidates[0];
            pending_commands.clear();
            if (winner == "L" || winner == "R") pending_commands.push_back("F");
            return winner;
        }

    }
    return std::nullopt;
}

std::optional<std::string> AI::UseTeleporter(const Percepts& percepts) {
    std::optional<Vec2> target_tel = DecideSeekTeleporter(percepts);//recomputed every turn
    if (target_tel.has_value()) {
        std::string action = MoveTowardTarget(target_tel.value(), percepts);
        if (action == "reached") return "U";
        return action;
    }
    return std::nullopt;
}

void AI::UpdateDeadEnds(const AdjacentCells& adj_cells, const Percepts& percepts) {
    bool backward_is_dead_end = dead_ends.count({ adj_cells.backward_cell.x, adj_cells.backward_cell.y }) > 0;
    bool left_is_wall = !percepts.left.empty() && percepts.left[0] == symbols.wall;
    bool right_is_wall = !percepts.right.empty() && percepts.right[0] == symbols.wall;
    if (backward_is_dead_end && left_is_wall && right_is_wall) {
        dead_ends.insert({ my_location.x, my_location.y });
    }
}

std::optional<std::string> AI::DecideExploration(const SafeDirections& safe) {
    Vec2 right_dir = Vec2(-my_heading.y, my_heading.x);
    Vec2 left_dir = Vec2(my_heading.y, -my_heading.x);

    int forward_unexplored;
    if (safe.forward_safe) forward_unexplored = CalculateUnexploredCells(my_heading);
    else forward_unexplored = -1;

    int right_unexplored;
    if (safe.right_safe) right_unexplored = CalculateUnexploredCells(right_dir);
    else right_unexplored = -1;

    int left_unexplored;
    if (safe.left_safe) left_unexplored = CalculateUnexploredCells(left_dir);
    else left_unexplored = -1;

    int least_unexplored = std::max({ forward_unexplored, right_unexplored, left_unexplored });

    std::vector<std::string> candidates;
    if (safe.forward_safe && forward_unexplored == least_unexplored) candidates.push_back("F");
    if (safe.right_safe && right_unexplored == least_unexplored) candidates.push_back("R");
    if (safe.left_safe && left_unexplored == least_unexplored) candidates.push_back("L");


    if (!candidates.empty()) {
        std::shuffle(candidates.begin(), candidates.end(), *rng);
        std::string winner = candidates[0];
        if (winner == "L" || winner == "R") pending_commands.push_back("F");
        return winner;
    }

    return std::nullopt;
}

std::string AI::FallBackDeadEnd() {
    dead_ends.insert({ my_location.x, my_location.y });
    pending_commands.push_back("R");
    pending_commands.push_back("F");
    return "R"; //??????????
}

std::string AI::DecideAction(const Percepts& percepts) {
	AdjacentCells adj_cells = GetAdjacentCells();
	SafeDirections safe = GetSafeDirections(percepts, adj_cells);

    //trap hunting mode
    if (auto a = TrapHunting(percepts, adj_cells)) return a.value();
	//treasure hunting mode
	if (auto a = TreasureHunting(percepts)) return a.value();
    //loop detection and breaking
    if (auto a = LoopDetection(safe)) return a.value();
   
    //pending commands
    if (!pending_commands.empty()) {
		std::string next_cmd = pending_commands.front();
        pending_commands.pop_front();
        return next_cmd;
    }
    
    //use or not a teleporter
	if (auto a = UseTeleporter(percepts)) return a.value();
    //TO BE ADDED: draw a new map
    //stores teleporters links

    
    //check for dead end cells
    UpdateDeadEnds(adj_cells, percepts);

    //explore mode
    //density scoring
    if (auto a = DecideExploration(safe)) return a.value();
    
    /*
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
    */
	//if no safe directions, mark current cell as dead end and return
    return FallBackDeadEnd();

}

std::vector<std::string> AI::Run(Percepts & percepts,AgentComm * comms) {
    current_turn++;
  std::cout << std::unitbuf;
  std::cout << "------------------------------------------------\n";
  std::cout << "AGENT ID: " << id << std::endl;
  PrintPercepts(percepts);

  //std::vector<std::string> cmds {"R", "B", "L", "F", "U", "D"};
  //std::shuffle(cmds.begin(), cmds.end(), *rng);
  //std::cout << "CMD:      " << cmds[0] << std::endl;
  std::cout << "max_turn: " << max_turn << " current_turn: " << current_turn << std::endl;  

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




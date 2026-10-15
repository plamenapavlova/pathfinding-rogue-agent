#include"ai.hpp"

/***************************************************************
AI CLASS DEFINITION
*/
AI::AI() { regions.push_back(RegionData()); }
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
{
    regions.push_back(RegionData());//new region data pushed back for this state
}

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
    auto& safe_cells = CurrentRegion().safe_cells;
    if (safe_cells.find({ loc.x, loc.y }) == safe_cells.end()) return false; return true;
}

void AI::MarkSafe(Vec2 loc) {
    auto& safe_cells = CurrentRegion().safe_cells;
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
    auto& my_location = CurrentRegion().my_location;
    AdjacentCells adj_cells;
	Directions dirs = GetDirections();

    adj_cells.forward_cell = my_location + dirs.forward;
	adj_cells.right_cell = my_location + dirs.right;
	adj_cells.left_cell = my_location + dirs.left;
	adj_cells.backward_cell = my_location + dirs.backward;

	return adj_cells;

}

AI::SafeDirections AI::GetSafeDirections(const Percepts& percepts, const AdjacentCells& adj_cells) {
    SafeDirections safe_dirs;
    auto& dead_ends = CurrentRegion().dead_ends;

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

AI::Directions AI::GetDirections() {
    auto& my_heading = CurrentRegion().my_heading;
    Directions dirs;
    dirs.forward = my_heading;
    dirs.right = Vec2(-my_heading.y, my_heading.x);
    dirs.left = Vec2(my_heading.y, -my_heading.x);
	dirs.backward = Vec2(-my_heading.x, -my_heading.y);
    return dirs;

}

void AI::UpdateLocation(std::string cmd) {
    auto& my_location = CurrentRegion().my_location;
    auto& my_heading = CurrentRegion().my_heading;

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
    auto& known_map = CurrentRegion().known_map;
    auto& visited_cells = CurrentRegion().visited_cells;
    auto& my_location = CurrentRegion().my_location;
    auto& my_heading = CurrentRegion().my_heading;

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
    auto& known_map = CurrentRegion().known_map;
    auto& my_location = CurrentRegion().my_location;

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

std::deque<std::string> AI::BFS(Vec2 target) {
    std::deque<std::string> commands;
    auto& my_location = CurrentRegion().my_location;
    auto& known_map = CurrentRegion().known_map;
    auto& my_heading = CurrentRegion().my_heading;

    if (my_location.x == target.x && my_location.y == target.y) return commands;

    std::vector<Vec2> directions = {Vec2(0, -1), Vec2(-1, 0), Vec2(0, 1), Vec2(1, 0)};
    std::deque<Vec2> queue;
    std::set<std::pair<int, int>> visited;
    std::map<std::pair<int, int>, std::pair<int, int>> came_from;

    queue.push_back(my_location);
    visited.insert({my_location.x, my_location.y});
    bool found = false;

    while (!queue.empty()) {
        Vec2 current = queue.front();
        queue.pop_front();
        auto current_pair = std::make_pair(current.x, current.y);
        
        if (current.x == target.x && current.y == target.y) {
            found = true;
            break;
        } 
        for (Vec2& dir : directions) {
            Vec2 next = current + dir;
            auto next_pair = std::make_pair(next.x, next.y);
            if (visited.find(next_pair) != visited.end()) continue;

            bool known_wall = (known_map.find(next_pair) != known_map.end() && known_map.find(next_pair)->second == symbols.wall);
            if (!known_wall) {
                queue.push_back(next);
                visited.insert(next_pair);
                came_from[next_pair] = current_pair;

            }

        }
    }

    if (!found) return commands;

    std::vector<Vec2> path;
    std::pair<int, int> cur = {target.x, target.y};
    while (!(cur.first == my_location.x && cur.second == my_location.y)) {
        path.push_back(Vec2(cur.first, cur.second));
        cur = came_from[cur];
    }
    std::reverse(path.begin(), path.end());

    Vec2 curr_location = my_location;
    Vec2 curr_heading = my_heading;
    for (auto& cell : path) {
        Vec2 diff = cell - curr_location;

        Vec2 right_of_current = Vec2(-curr_heading.y, curr_heading.x);
        Vec2 left_of_current = Vec2(curr_heading.y, -curr_heading.x);
        Vec2 behind_current = Vec2(-curr_heading.x, -curr_heading.y);
        if(curr_heading== diff)  commands.push_back("F");
        else if (right_of_current == diff) {
            commands.push_back("R");
            commands.push_back("F");
            curr_heading = right_of_current;
        }
        else if (left_of_current == diff) {
            commands.push_back("L");
            commands.push_back("F");
            curr_heading = left_of_current;
        }
        else if (behind_current ==diff) {
            commands.push_back("R");
            commands.push_back("R");
            commands.push_back("F");
            curr_heading = behind_current;
        }
        curr_location = cell;
    }

    return commands;


}

std::string AI::MoveTowardTarget(Vec2 target, const Percepts& percepts) {
    auto& my_location = CurrentRegion().my_location;
    if (my_location.x == target.x && my_location.y == target.y) return "reached";

    if (pending_commands.empty()) {
        pending_commands = BFS(target);
        if (pending_commands.empty()) return "unreachable";
    }
    
    std::string next_cmd = pending_commands.front();
    pending_commands.pop_front();
    return next_cmd;

}

int AI::CalculateUnexploredCells(Vec2 direction) {
    auto& my_location = CurrentRegion().my_location;
    auto& known_map = CurrentRegion().known_map;

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
    auto& my_location = CurrentRegion().my_location;

	Directions dirs = GetDirections();
    static const double WAIT_FRACTION = 0.3;

    int remaining_turns = max_turn - current_turn;
    int tel_min_turns = int(region_remaining_turns * WAIT_FRACTION);


    std::optional<Vec2> nearest_known_tel = std::nullopt;

    int shortest_dist = std::numeric_limits<int>::max();
    int best_ranking = 3;

    for (const std::string& tel_symbol : symbols.teleporters) {
        std::optional<Vec2> candidate = FindNearestObject(tel_symbol);
        if (candidate.has_value()) { 
            int distance = my_location.ManhattanDistance(candidate.value());

            int ranking;
            if (used_teleporters.count(tel_symbol) > 0) ranking = 3;
            else if (headed_to_teleporters.count(tel_symbol) > 0 || reached_teleporters.count(tel_symbol) > 0) ranking = 2;
            else ranking = 1;
            std::cout << "[RANK DEBUG] symbol='" << tel_symbol << "' dist=" << distance << " ranking=" << ranking << std::endl;
            if (ranking<best_ranking || (ranking ==best_ranking && distance < shortest_dist)) {
				best_ranking = ranking;
                shortest_dist = distance;
                nearest_known_tel = candidate;
            }

        }
    }
    
    if (nearest_known_tel.has_value())
        std::cout << "[RANK DEBUG] CHOSEN dist=" << shortest_dist << " ranking=" << best_ranking << std::endl;
    if (!nearest_known_tel.has_value()) {
        std::cout << "[TELE DEBUG] No teleporter found in known_map at all." << std::endl;
        return std::nullopt;
    }
    int tel_dist = my_location.ManhattanDistance(nearest_known_tel.value());
    std::cout << "[TELE DEBUG] nearest teleporter at (" << nearest_known_tel.value().x
        << "," << nearest_known_tel.value().y << ") tel_dist=" << tel_dist << std::endl;

    if (tel_dist >= remaining_turns) return std::nullopt;
    std::cout << "[TELE DEBUG] Turn-window check PASSED." << std::endl;
	

    if (turns_in_region < tel_min_turns) {
        std::cout << "[TELE DEBUG] Still in explore window, not seeking yet." << std::endl;
        return std::nullopt;
    }
    if (best_ranking == 1) {
        return nearest_known_tel;
        //add some threshold before using a teleporter again
    }
    

    int forward_unexplored = CalculateUnexploredCells(dirs.forward);
    int right_unexplored = CalculateUnexploredCells(dirs.right);
    int left_unexplored = CalculateUnexploredCells(dirs.left);
    int backward_unexplored = CalculateUnexploredCells(dirs.backward);
	int total_unexplored = forward_unexplored + right_unexplored + left_unexplored + backward_unexplored;
    int threshold = 5;
    if (total_unexplored > threshold) {
		return std::nullopt;
    }
    
    return nearest_known_tel;

    
}

std::optional<std::string> AI::TrapHunting(const Percepts& percepts, const AdjacentCells& adj_cells) {
    auto& safe_cells = CurrentRegion().safe_cells;

    if (percepts.detector == 1 && safe_cells.find({ adj_cells.forward_cell.x, adj_cells.forward_cell.y }) == safe_cells.end()) {
        pending_disarm_cell = adj_cells.forward_cell;
        return "D";
    }
    return std::nullopt;
}

std::optional<std::string> AI::TreasureHunting(const Percepts& percepts) {
    auto& current_goal = CurrentRegion().current_goal;
    auto& known_map = CurrentRegion().known_map;
    auto& my_location = CurrentRegion().my_location;

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
        if (action == "unreachable") {
			current_goal = std::nullopt;
			return std::nullopt;
        }
        return action;
    }
    return std::nullopt;
}
std::optional<std::string> AI::LoopDetection(const SafeDirections& safe) {
    auto& last_visited_cells = CurrentRegion().last_visited_cells;
    auto& my_location = CurrentRegion().my_location;

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
        comms->SetSignal(id, EncodeSignal(target_tel.value(), 1));
        std::string action = MoveTowardTarget(target_tel.value(), percepts);
        if (action == "reached") {
            comms->SetSignal(id, EncodeSignal(target_tel.value(), 2));
            std::string symbol = percepts.current[0]; 
			pending_teleporter = symbol;
            used_teleporters.insert(symbol);
            if (symbol_to_region_id.find(symbol) == symbol_to_region_id.end())
                symbol_to_region_id[symbol] = current_region_id;

            pending_commands.clear();
            pending_disarm_cell = std::nullopt;
            turns_in_region = 0;
            region_remaining_turns = max_turn - current_turn;
            return "U";
        }
        if (action == "unreachable") {
            return std::nullopt;
        }
        return action;
    }
    return std::nullopt;
}

bool AI::WallOrDead(Vec2 cell) {
    auto& known_map = CurrentRegion().known_map;
    auto& dead_ends = CurrentRegion().dead_ends;

    if (known_map.find({ cell.x, cell.y }) != known_map.end() && known_map.find({ cell.x, cell.y })->second == symbols.wall
        ||
        dead_ends.find({ cell.x, cell.y }) != dead_ends.end()) return true;
    return false;
}

void AI::UpdateDeadEnds(const AdjacentCells& adj_cells) {
    auto& dead_ends = CurrentRegion().dead_ends;
    auto& my_location = CurrentRegion().my_location;

    int count = 0;

    if (WallOrDead(adj_cells.forward_cell)) count++;
    if (WallOrDead(adj_cells.right_cell)) count++;
    if (WallOrDead(adj_cells.left_cell)) count++;
    if (WallOrDead(adj_cells.backward_cell)) count++;

    if (count >= 3) {
        dead_ends.insert({ my_location.x, my_location.y });
    }
}


std::optional<std::string> AI::DecideExploration(const SafeDirections& safe, const Percepts& percepts) {
	Directions dirs = GetDirections();

    int forward_unexplored;
    if (safe.forward_safe) forward_unexplored = CalculateUnexploredCells(dirs.forward);
    else forward_unexplored = -1;

    int right_unexplored;
    if (safe.right_safe) right_unexplored = CalculateUnexploredCells(dirs.right);
    else right_unexplored = -1;

    int left_unexplored;
    if (safe.left_safe) left_unexplored = CalculateUnexploredCells(dirs.left);
    else left_unexplored = -1;


    bool forward_occupied = false;
	bool left_occupied = false;
	bool right_occupied = false;

    for (auto& agent_dist : percepts.others) {
        if (agent_dist.x == 0 && agent_dist.y == 0) continue;//skip self
        if (std::abs(agent_dist.x) >= std::abs(agent_dist.y)) {
            if (agent_dist.x > 0) right_occupied = true;
            else left_occupied = true;
        }
        else {
            if (agent_dist.y >= 0) forward_occupied = true;
			
        }
    }


    std::vector<std::string> prefferable_dirs;
    if (!forward_occupied && safe.forward_safe && forward_unexplored > 1) prefferable_dirs.push_back("F");
    if (!left_occupied && safe.left_safe && left_unexplored > 1) prefferable_dirs.push_back("L");
    if (!right_occupied && safe.right_safe && right_unexplored > 1) prefferable_dirs.push_back("R");

    if (!prefferable_dirs.empty()) {
        std::shuffle(prefferable_dirs.begin(), prefferable_dirs.end(), *rng);
        std::string winner = prefferable_dirs[0];
        if (winner == "L" || winner == "R") pending_commands.push_back("F");
        return winner;
    }

    int least_unexplored = std::max({ forward_unexplored, right_unexplored, left_unexplored });

    std::vector<std::string> backups;
    if (safe.forward_safe && forward_unexplored == least_unexplored) backups.push_back("F");
    if (safe.right_safe && right_unexplored == least_unexplored) backups.push_back("R");
    if (safe.left_safe && left_unexplored == least_unexplored) backups.push_back("L");


    if (!backups.empty()) {
        std::shuffle(backups.begin(), backups.end(), *rng);
        std::string winner = backups[0];
        if (winner == "L" || winner == "R") pending_commands.push_back("F");
        return winner;
    }

    return std::nullopt;
}

std::string AI::FallBackDeadEnd() {
    auto& dead_ends = CurrentRegion().dead_ends;
    auto& my_location = CurrentRegion().my_location;

    dead_ends.insert({ my_location.x, my_location.y });
    pending_commands.push_back("R");
    pending_commands.push_back("F");
    return "R"; //??????????
}

void AI::MarkMapSafe() {
    auto& known_map = CurrentRegion().known_map;

    for (const auto& cell : known_map) {
        const std::string& symbol = cell.second;
        if (symbol != symbols.wall) {
            MarkSafe(Vec2(cell.first.first, cell.first.second));
        }
    }
}

AI::RegionData& AI::CurrentRegion() {
    return regions[current_region_id];
}


unsigned AI::EncodeSignal(Vec2 tele_coor, int status) {
    //status: 1- going to the teleporter, 2 - getting the teleporter
	auto& known_map = CurrentRegion().known_map;
    std::string tele_symbol;
    tele_symbol = known_map[{tele_coor.x, tele_coor.y}];

    
    int size = symbols.teleporters.size();
    unsigned signal = 0;
    unsigned place = 1;
    for (int i = 0; i < size; i++) {
        if (symbols.teleporters[i] == tele_symbol) {
            signal += status * place;
        }
        else {
            place *= 10;
        }
    }

    return signal;

}

void AI::DecodeSignal(unsigned signal) {
    unsigned pos = 1;
    for (size_t i = 0; i < symbols.teleporters.size(); i++) {
        int digit = (signal / pos) % 10;
        std::string symbol = symbols.teleporters[i];
        if (digit == 1) headed_to_teleporters.insert(symbol); 
		else if (digit == 2) reached_teleporters.insert(symbol);

        pos *= 10;
    }

}


std::string AI::DecideAction(const Percepts& percepts) {
	AdjacentCells adj_cells = GetAdjacentCells();
	SafeDirections safe = GetSafeDirections(percepts, adj_cells);

    //trap hunting mode
    if (auto a = TrapHunting(percepts, adj_cells)) return a.value();
	//treasure hunting mode
	if (auto a = TreasureHunting(percepts)) return a.value();
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
    // 
    //loop detection and breaking
    if (auto a = LoopDetection(safe)) return a.value();
    
    //check for dead end cells
    UpdateDeadEnds(adj_cells);

    //explore mode
    //density scoring
    if (auto a = DecideExploration(safe, percepts)) return a.value();
    
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

std::vector<std::string> AI::Run(Percepts& percepts, AgentComm* comms) {
    current_turn++;
    turns_in_region++;
    this->comms = comms;
    std::cout << std::unitbuf;
    std::cout << "------------------------------------------------\n";
    std::cout << "AGENT ID: " << id << std::endl;
    PrintPercepts(percepts);

    //std::vector<std::string> cmds {"R", "B", "L", "F", "U", "D"};
    //std::shuffle(cmds.begin(), cmds.end(), *rng);
    //std::cout << "CMD:      " << cmds[0] << std::endl;
    std::cout << "max_turn: " << max_turn << " current_turn: " << current_turn << std::endl;


    if (pending_teleporter.has_value()) {
		std::string arr_symbol = percepts.current[0];
        std::cout << "[REGION DEBUG] departed via '" << pending_teleporter.value()
            << "' arrived on '" << arr_symbol << "'";
        teleporter_pairs[pending_teleporter.value()] = arr_symbol;
        if (symbol_to_region_id.find(arr_symbol) != symbol_to_region_id.end()) {
            current_region_id = symbol_to_region_id[arr_symbol];
            std::cout << " -> RECOGNIZED region " << current_region_id << std::endl;
        }
        else {
            regions.push_back(RegionData());
            current_region_id = (int)regions.size() - 1;
            symbol_to_region_id[arr_symbol] = current_region_id;

            std::cout << " -> NEW region " << current_region_id << std::endl;
        }

		pending_teleporter = std::nullopt;
    }

  UpdateMap(percepts);
  auto& known_map = CurrentRegion().known_map;
  auto& my_location = CurrentRegion().my_location;
  auto& dead_ends = CurrentRegion().dead_ends;
  auto& safe_cells = CurrentRegion().safe_cells;

  std::cout << "Known map (" << known_map.size() << " cells):\n";
  for (const auto& entry : known_map) {
      std::cout << "  (" << entry.first.first << "," << entry.first.second
          << "): '" << entry.second << "'\n";
  }


  if (pending_disarm_cell.has_value()) {
      MarkSafe(pending_disarm_cell.value());
      pending_disarm_cell = std::nullopt;
  }

  
  if (percepts.detector == -1) MarkMapSafe();//no more traps left
  else {
      int trap_dist = percepts.detector;
      SafeZone(my_location, trap_dist);
  }
    
  std::cout << "Safe cells: ";
  for (const auto& cell : safe_cells) {
      std::cout << "(" << cell.first << "," << cell.second << ") ";
  }
  std::cout << std::endl;

  
  std::cout << "Dead ends: ";
  for (const auto& cell : dead_ends) {
      std::cout << "(" << cell.first << "," << cell.second << ") ";
  }
  std::cout << std::endl;

  std::cout << "Current region: " << current_region_id << " (total regions: " << regions.size() << ")" << std::endl;
  std::cout << "Symbol->region map: ";
  for (const auto& entry : symbol_to_region_id) {
      std::cout << "[" << entry.first << "->" << entry.second << "] ";
  }
  std::cout << std::endl;

  std::cout << "My location: (" << CurrentRegion().my_location.x << "," << CurrentRegion().my_location.y
      << ") heading: (" << CurrentRegion().my_heading.x << "," << CurrentRegion().my_heading.y << ")" << std::endl;


  headed_to_teleporters.clear();
  int num_agents = comms->GetNumAgents();
  for (int i = 0; i < num_agents; i++) {
      if (i == (int) id) continue;
      unsigned signal = comms->GetSignal(i);
      DecodeSignal(signal);
  }

  std::cout << "[SETS] headed_to: ";
  for (auto& s : headed_to_teleporters) std::cout << s << " ";
  std::cout << "| reached: ";
  for (auto& s : reached_teleporters) std::cout << s << " ";
  std::cout << "| used: ";
  for (auto& s : used_teleporters) std::cout << s << " ";
  std::cout << std::endl;
 

  std::string action = DecideAction(percepts);
  std::cout << "Action: " << action << std::endl;
  UpdateLocation(action);
  return {action};

}




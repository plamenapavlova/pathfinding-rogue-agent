# Multi-Agent Exploration & Decision-Making System (C++17)

Autonomous AI agents that explore an unknown, partially observable grid maze without a map. They build an internal model of the world, disarm hidden traps, collect treasure, use teleporters, and coordinate with each other to avoid duplicating effort.

> **Attribution**
> This project was built for **COS4070 (Artificial Intelligence)** at the American University in Bulgaria.
> The simulation environment, **[Microworld2](https://github.com/zaxhutchinson/Microworld2)**, was provided by **Prof. Z. Hutchinson**. It includes the world engine, rendering, percept and command interfaces, scoring, and test maps.
> **My contribution is the AI itself** (`ai.hpp` / `ai.cpp`): all of the logic that decides how each agent perceives, remembers, plans, and acts. As the assignment required, none of the simulator files were modified.

---

## The Problem

Agents ("rogues") start at random positions with random headings and must act under tight constraints:

- **Partial observability.** An agent sees a few cells forward, backward, left and right. Walls block vision.
- **Invisible traps.** Traps look like open floor. The only clue is the distance to the nearest trap.
- **No global orientation.** There is no north. Every percept is relative to the agent's own heading.
- **Teleporters** move an agent to an unknown location, possibly with no way back.
- **Limited communication.** Agents cannot see each other. They share only relative distances and a small shared message buffer.
- **Unknown configuration.** Map, number of agents, sight range, and trap and teleporter layout change between runs.

### PEAS

| | |
|---|---|
| **Performance** | Collect treasure, disarm traps, stay alive, minimize wasted actions, communicate, use all percepts |
| **Environment** | Grid maze with walls, traps, treasure, teleporters, and other agents |
| **Actuators** | Move forward/backward, turn left/right, use teleporter, disarm, take treasure |
| **Sensors** | Limited line-of-sight vision, trap distance, distance to other agents |

**Environment class:** partially observable, cooperative multi-agent, deterministic (optionally stochastic via `slip_chance`), sequential, dynamic, discrete, known.

## Agent Architecture

The agent is **goal-based on a model-based core**, with a **reflex safety layer** and **utility-style scoring** for exploration.

### World model
Each agent keeps an internal state that is updated every turn from relative percepts:

- `known_map`: every cell the agent has seen
- `visited_cells`, `safe_cells`, `dead_ends`: where it has been, what is proven safe, and which paths lead nowhere
- `my_location`, `my_heading`: its own pose in a self-defined coordinate frame

These are updated by `UpdateMap`, `UpdateLocation`, `MarkSafe`, and `SafeZone`. After a teleport, the agent starts a **fresh coordinate frame**, because its old position is no longer valid.

### Decision priority (evaluated every turn)

1. **Safety (reflex):** `TrapHunting` runs first. If the cell ahead isn't proven safe and the trap-distance sensor shows a trap could be there, the agent disarms it before stepping forward.
2. **Collect treasure:** when treasure is visible, `TreasureHunting` sets it as the goal.
3. **Finish a committed plan:** queued `pending_commands` from an earlier decision are carried out, so the agent doesn't flip-flop.
4. **Seek a teleporter:** only after the reachable area is fully explored (`DecideSeekTeleporter`).
5. **Break loops:** if the agent has revisited the same cells several times recently, it forces a new move.
6. **Explore:** `DecideExploration` scores each direction by the number of reachable unexplored cells (`CalculateUnexploredCells`) and penalizes directions where other agents already are.
7. **Dead-end fallback:** when only one way out remains, the agent takes it and marks the dead end.

### Path planning
`MoveTowardTarget` plans a route to the current goal (treasure, frontier, or teleporter) with **Breadth-First Search** over the internal map, and turns that route into a sequence of turn and move commands.

### Multi-agent coordination
- **Teleporter claiming:** agents use the shared message buffer to announce which teleporter they are heading to or taking, so two agents don't race for the same exit.
- **Spreading out:** exploration directions with other agents nearby (from rogue distances) score lower, so agents cover different areas.
- The design works with **any number of agents**, including a single agent.

## Results

Tested on the provided worlds, custom-built maps, and randomized start positions and headings:

- Agents **collect at least 90% of the treasure** given enough turns.
- Agents **disarm traps whenever a trap may lie on their path**.
- The system scales to any number of agents and arbitrary teleporter layouts.


## Design Evolution

The exploration strategy went through several iterations, each driven by a failure observed in the previous one:

| Version | Strategy | Problem found |
|---|---|---|
| 1 | Go forward; turn right on wall | Missed any treasure not on the forward path |
| 2 | Random safe direction | Stayed in one area, with no purposeful exploration |
| 3 | Prefer unvisited neighbor cells | One-cell check too narrow; revisited areas via different paths |
| 4 | Score whole directions by unexplored area (`known_map`, not just walked cells) | Counted cells hidden behind walls, which caused a deterministic loop |
| 5 | Count only **reachable** unexplored cells, add commitment, loop-breaking, and teleporter coordination | Current version |

**Debugging example.** Version 4 produced a stable 8-step loop. The direction score kept counting unreachable cells behind walls, so the same wrong choice won every time. Logging the scores per step (position, heading, F/R/L values, choice) exposed the cycle. Excluding cells behind walls revealed that the nearby area was already explored, so the only meaningful move was outward.

Other lessons learned:
- Moving backward is dangerous, because the agent can only disarm the cell it is facing.
- Teleporters need a new map frame after each jump. Without one, agents bounced between teleporters forever.
- Refactoring into small single-purpose functions made it much easier to add new behavior layers.

## Known Limitations

- **Direction oscillation:** greedy direction scoring can still occasionally switch back and forth before committing.
- **Large treasure-free areas:** the agent fully explores before teleporting, by design, so it doesn't abandon reachable treasure. On large empty maps it can run out of turns.
- **BFS treats unseen cells as walkable,** so plans may need to change once new walls are seen.
- **Memory:** the map structures grow with explored area and could be compacted for very large worlds.


| Flag | Meaning |
|---|---|
| `-w <name>` | World file from the `worlds/` folder |
| `-rsh` | Random start positions and headings |
| `-d <ms>` | Delay between agent updates |
| `-t <n>` | Maximum number of turns |
| `-s <seed>` | Fixed seed for reproducible runs |

## Tech

C++17 · STL · Breadth-First Search · Model-based / goal-based agent design · Reasoning under uncertainty · Multi-agent coordination · Heuristic scoring

## Credits

- **AI logic (`ai.hpp`, `ai.cpp`):** Plamena Pavlova
- **Simulator (Microworld2):** Prof. Z. Hutchinson, AUBG
- BFS implementation adapted from GeeksforGeeks, with additional features

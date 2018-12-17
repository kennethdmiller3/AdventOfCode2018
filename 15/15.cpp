#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <algorithm>

// https://adventofcode.com/2018/day/15

enum class Team : uint8_t
{
	Elf,
	Goblin,
	Count
};

struct Unit
{
	uint16_t pos;
	uint8_t id;
	Team team;
	uint8_t power;
	uint8_t health;
};

uint8_t ManhattanDistance(uint16_t p0, uint16_t p1, uint8_t width)
{
	uint8_t y0(p0 / width);
	uint8_t x0(p0 - y0 * width);
	uint8_t y1(p1 / width);
	uint8_t x1(p1 - y1 * width);
	return abs(y1 - y0) + abs(x1 - x0);
}

char const TEAM_NAME[int(Team::Count)] = { 'E', 'G' };
char const DIR_NAME[4] = { '^', '<', '>', 'v' };

void ReadLayout(std::string &layout, uint8_t &width, uint8_t &height, std::istream &input)
{
	layout.clear();
	width = 0;
	height = 0;

	// read the initial layout
	std::string line;
	while (std::getline(input, line))
	{
		layout.append(line);
		layout.push_back('\n');

		width = std::max(width, uint8_t(line.size() + 1));
		++height;
	}
}

Team Simulate(uint32_t &rounds, std::vector<Unit> &unit_by_id, std::string &layout, int8_t width, int8_t height, uint8_t elf_strength, bool allow_elf_casualties)
{
	unit_by_id.clear();

	// find the combatants
	uint16_t pos = 0;
	uint8_t next_id = 0;
	for (char const c : layout)
	{
		switch (c)
		{
		case 'E': unit_by_id.push_back({ pos, next_id++, Team::Elf, elf_strength, 200 }); break;
		case 'G': unit_by_id.push_back({ pos, next_id++, Team::Goblin, 3, 200 }); break;
		}
		++pos;
	}

	// build a map of the id of the unit at each pos (or UINT8_MAX for none)
	std::vector<uint8_t> id_at_pos(width * height, UINT8_MAX);
	for (Unit const &unit : unit_by_id)
		id_at_pos[unit.pos] = unit.id;

	// direction offset table
	const short offset[4] = { -width, -1, 1, width };

	// A* path planning data
	std::vector<uint16_t> goal_at_pos(width * height, UINT16_MAX);
	std::vector<uint8_t> g_at_pos(width * height, UINT8_MAX);
	std::vector<uint8_t> f_at_pos(width * height, UINT8_MAX);
	std::vector<bool> open_at_pos(width * height, false);
	std::vector<bool> closed_at_pos(width * height, false);

	// A* path planning open set
	std::vector<uint16_t> open_set;
	open_set.reserve(width * height);

	// predicate for comparing two locations in the A* open set
	auto OpenSetPredicate = [&goal_at_pos, &g_at_pos, &f_at_pos](uint16_t a, uint16_t b) -> bool
	{
		if (f_at_pos[a] > f_at_pos[b])
			return true;
		if (f_at_pos[a] < f_at_pos[b])
			return false;
		if (g_at_pos[a] > g_at_pos[b])
			return true;
		if (g_at_pos[a] < g_at_pos[b])
			return false;
		if (goal_at_pos[a] > goal_at_pos[b])
			return true;
		if (goal_at_pos[a] < goal_at_pos[b])
			return false;
		return a > b;
	};

	// unit initiative order list
	std::vector<uint8_t> id_by_order;
	id_by_order.reserve(unit_by_id.size());

#ifdef DEBUG_OUTPUT
	// initial layout
	std::cout << "Initial\n";
	std::cout << layout;
#endif

	// for each round...
	rounds = 0;
	for (;;)
	{
#ifdef DEBUG_OUTPUT
		std::cout << "Round " << rounds + 1 << "\n";
#else
		std::cout << '.';
#endif

		// count the number of units on each team
		// and build the unit initiative order list
		id_by_order.clear();
		int count[int(Team::Count)] = { 0, 0 };
		for (Unit const &unit : unit_by_id)
		{
			if (unit.health > 0)
			{
				count[int(unit.team)]++;
				id_by_order.push_back(unit.id);
			}
		}
		std::sort(id_by_order.begin(), id_by_order.end(),
			[&unit_by_id](uint8_t a, uint8_t b) -> bool { return unit_by_id[a].pos < unit_by_id[b].pos;  });

		// for each active unit...
		for (uint8_t unit_id : id_by_order)
		{
			// get the unit
			Unit &unit = unit_by_id[unit_id];

			// skip if dead
			if (unit.health == 0)
				continue;

			// stop if there's no one left on the other team to target
			if (count[1 - int(unit.team)] == 0)
			{
				// return the winning team
				return unit.team;
			}

			// unit's attack target
			uint8_t attack_target_id = UINT8_MAX;

			// check if there's an enemy already in range
			uint8_t best_health = UINT8_MAX;
			for (int dir = 0; dir < 4; ++dir)
			{
				uint16_t neighbor_pos = unit.pos + offset[dir];
				uint8_t id = id_at_pos[neighbor_pos];
				if (id != UINT8_MAX)
				{
					Unit const &target = unit_by_id[id];
					if (target.team != unit.team &&
						best_health > target.health)
					{
						best_health = target.health;
						attack_target_id = id;
					}
				}
			}

			// no adjacent target; need to find where to move
			if (attack_target_id == UINT8_MAX)
			{
				// initialize the path planning data
				std::fill(goal_at_pos.begin(), goal_at_pos.end(), UINT16_MAX);
				std::fill(g_at_pos.begin(), g_at_pos.end(), UINT8_MAX);
				std::fill(f_at_pos.begin(), f_at_pos.end(), UINT8_MAX);
				std::fill(open_at_pos.begin(), open_at_pos.end(), false);
				std::fill(closed_at_pos.begin(), closed_at_pos.end(), false);

				// add goal nodes to the open set
				open_set.clear();
				for (Unit const &target : unit_by_id)
				{
					if (target.team == unit.team)
						continue;
					if (target.health == 0)
						continue;

					for (int dir = 0; dir < 4; ++dir)
					{
						uint16_t neighbor_pos = target.pos + offset[dir];

						// skip if blocked
						if (layout[neighbor_pos] != '.')
							continue;

						// skip if open
						if (open_at_pos[neighbor_pos])
							continue;

						// add location
						goal_at_pos[neighbor_pos] = neighbor_pos;
						g_at_pos[neighbor_pos] = 0;
						f_at_pos[neighbor_pos] = ManhattanDistance(unit.pos, neighbor_pos, width);
						open_at_pos[neighbor_pos] = true;

						// add to open set
						open_set.push_back(neighbor_pos);
						std::push_heap(open_set.begin(), open_set.end(), OpenSetPredicate);
					}
				}

				while (!open_set.empty())
				{
					// get the node with the best score
					std::pop_heap(open_set.begin(), open_set.end(), OpenSetPredicate);
					uint16_t current_pos = open_set.back();
					open_set.pop_back();

					// remove from the open set
					open_at_pos[current_pos] = false;

					// add to the closed set
					closed_at_pos[current_pos] = true;

					// goal identifier for the path (for breaking ties)
					uint16_t goal_pos = goal_at_pos[current_pos];

					// distance at the current position
					uint8_t g_at_current = g_at_pos[current_pos];

					// for each neighbor...
					for (int dir = 0; dir < 4; ++dir)
					{
						uint16_t neighbor_pos = current_pos + offset[dir];

						// skip if closed
						if (closed_at_pos[neighbor_pos])
							continue;

						// skip if blocked
						if (layout[neighbor_pos] != '.')
							continue;

						// tentative distance at the neighbor
						uint8_t g = g_at_current + 1;

						// if the new value is better...
						if (g < g_at_pos[neighbor_pos] || (g == g_at_pos[neighbor_pos] && goal_pos < goal_at_pos[neighbor_pos]))
						{
							// update the path data
							goal_at_pos[neighbor_pos] = goal_pos;
							g_at_pos[neighbor_pos] = g;
							f_at_pos[neighbor_pos] = g + ManhattanDistance(unit.pos, neighbor_pos, width);

							if (!open_at_pos[neighbor_pos])
							{
								// discovered a new node
								open_at_pos[neighbor_pos] = true;
								open_set.push_back(neighbor_pos);
								std::push_heap(open_set.begin(), open_set.end(), OpenSetPredicate);
							}
						}
					}
				}

				// choose the best direction to move
				uint16_t best_goal = UINT16_MAX;
				uint8_t best_g = UINT8_MAX;
				int best_dir = -1;
				for (int dir = 0; dir < 4; ++dir)
				{
					uint16_t neighbor_pos = unit.pos + offset[dir];

					// skip if blocked
					if (layout[neighbor_pos] != '.')
						continue;

					// update the best direction
					uint8_t g = g_at_pos[neighbor_pos];
					uint16_t goal = goal_at_pos[neighbor_pos];
					if (g < best_g || (g == best_g && goal < best_goal))
					{
						best_goal = goal;
						best_g = g;
						best_dir = dir;
					}
				}

#ifdef DEBUG_OUTPUT
				std::cout << TEAM_NAME[int(unit.team)] << "#" << int(unit_id) << " @" << unit.pos % width << "," << unit.pos / width;
#endif

				if (best_dir >= 0)
				{
					// exit the old location
					layout[unit.pos] = '.';
					id_at_pos[unit.pos] = UINT8_MAX;

					// move in the direction
					unit.pos += offset[best_dir];

					// enter the new location
					layout[unit.pos] = TEAM_NAME[int(unit.team)];
					id_at_pos[unit.pos] = unit.id;

#ifdef DEBUG_OUTPUT
					std::cout << " moves " << DIR_NAME[best_dir] << " to @" << unit.pos % width << "," << unit.pos / width << "\n";
#endif

					// update the target
					best_health = UINT8_MAX;
					for (int dir = 0; dir < 4; ++dir)
					{
						uint16_t neighbor_pos = unit.pos + offset[dir];
						uint8_t id = id_at_pos[neighbor_pos];
						if (id != UINT8_MAX)
						{
							Unit const &target = unit_by_id[id];
							if (target.team != unit.team &&
								best_health > target.health)
							{
								best_health = target.health;
								attack_target_id = id;
							}
						}
					}
				}
#ifdef DEBUG_OUTPUT
				else
				{
					std::cout << " can't move\n";
				}
#endif
			}

			// if the unit has a target to attack...
			if (attack_target_id != UINT8_MAX)
			{
				Unit &target = unit_by_id[attack_target_id];

#ifdef DEBUG_OUTPUT
				std::cout << TEAM_NAME[int(unit.team)] << "#" << int(unit_id) << " @" << unit.pos % width << "," << unit.pos / width;
				std::cout << " attacks " << TEAM_NAME[int(target.team)] << "#" << int(attack_target_id) << " @" << target.pos % width << "," << target.pos / width;
#endif

				if (unit.power >= target.health)
				{
					// target died
					target.health = 0;

					// clear out the location
					layout[target.pos] = '.';
					id_at_pos[target.pos] = UINT8_MAX;

					// reduce the team count
					--count[int(target.team)];

					if (!allow_elf_casualties && target.team == Team::Elf)
					{
						// immediately fail
						return Team::Goblin;
					}
				}
				else
				{
					// apply damage
					target.health -= unit.power;
				}

#ifdef DEBUG_OUTPUT
				std::cout << " health=" << int(target.health) << "\n";
#endif
			}
		}

#ifdef DEBUG_OUTPUT
		std::cout << layout;
#endif

		// completed a round
		++rounds;
	}

	// shouldn't happen
	return Team::Count;
}

void Part1(std::string const &initial, uint8_t width, uint8_t height)
{
	// active units indexed by identifier
	std::vector<Unit> unit_by_id;

	std::cout << "Part 1:\n";

	// simulate the battle
	std::string layout = initial;
	uint32_t rounds;
	Team winner = Simulate(rounds, unit_by_id, layout, width, height, 3, true);

	// print the outcome
#ifdef DEBUG_OUTPUT
	std::cout << layout;
#endif
	std::cout << "\nCombat ends after " << rounds << " full rounds\n";
	uint16_t total_health = 0;
	for (Unit const &unit : unit_by_id)
	{
		total_health += unit.health;
	}
	std::cout << TEAM_NAME[int(winner)] << " win with " << total_health << " total hit points left\n";
	std::cout << "Outcome: " << rounds << " * " << total_health << " = " << rounds * total_health << "\n\n";
}

void Part2(std::string const &initial, uint8_t width, uint8_t height)
{
	// active units indexed by identifier
	std::vector<Unit> unit_by_id;

	std::string layout;
	uint8_t elf_strength;
	uint32_t rounds;
	Team winner;

	std::cout << "Part 2:\n";

	// find the lowest elf strength that leads to victory with no elf casualties
	for (elf_strength = 3; elf_strength < UINT8_MAX; ++elf_strength)
	{
		// simulate the battle
		std::cout << "Strength=" << int(elf_strength) << " ";
		layout = initial;
		winner = Simulate(rounds, unit_by_id, layout, width, height, elf_strength, false);
		if (winner == Team::Elf)
		{
			std::cout << "VICTORY!\n";
			break;
		}
		std::cout << "DEFEAT!\n";
	}

	// print the outcome
	std::cout << "\nCombat ends after " << rounds << " full rounds\n";
	uint16_t total_health = 0;
	for (Unit const &unit : unit_by_id)
	{
		total_health += unit.health;
	}
	std::cout << TEAM_NAME[int(winner)] << " win with " << total_health << " total hit points left\n";
	std::cout << "Outcome: " << rounds << " * " << total_health << " = " << rounds * total_health << "\n\n";
}

int main()
{
	// read the initial layout
	std::string initial;
	uint8_t width = 0, height = 0;
	ReadLayout(initial, width, height, std::cin);

	Part1(initial, width, height);
	Part2(initial, width, height);

	return 0;
}
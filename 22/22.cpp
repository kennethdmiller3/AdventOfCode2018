#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

// https://adventofcode.com/2018/day/22

#define DEBUG_PRINT 0

using CoordinateType = uint16_t;
using ErosionType = uint32_t;
using CostType = uint32_t;

enum class RegionType : int8_t
{
	Rocky = 0,
	Wet = 1,
	Narrow = 2,
	Count = 3
};

enum class ToolType : int8_t
{
	Neither = 0,
	Torch = 1,
	Climbing = 2,
	Count = 3
};

struct Position
{
	CoordinateType x;
	CoordinateType y;
};

inline bool operator<(Position const &a, Position const &b)
{
	if (a.y < b.y)
		return true;
	if (a.y > b.y)
		return false;
	return a.x < b.x;
}
inline bool operator==(Position const &a, Position const &b)
{
	return a.x == b.x && a.y == b.y;
}

using StateType = std::pair<Position, ToolType>;

namespace std
{
	template<>
	struct hash<Position>
	{
		inline std::size_t operator()(Position const &p) const
		{
			return hash<uint32_t>()((uint32_t(p.y) << 16) + p.x);
		}
	};
	template<>
	struct hash<StateType>
	{
		inline std::size_t operator()(StateType const &s) const
		{
			return hash<Position>()(s.first) ^ hash<uint8_t>()(uint8_t(s.second));
		}
	};
}

using ErosionMap = std::unordered_map<Position, ErosionType>;

ErosionType GetErosionLevel(ErosionMap &erosion_levels, Position const &position, Position const &target, uint32_t depth)
{
	// if already computed, return the cached result
	ErosionMap::iterator itor = erosion_levels.find(position);
	if (itor != erosion_levels.end())
		return itor->second;

	// compute the geo index for the location
	uint32_t geo_index = 0;
	if (position.x == 0 && position.y == 0)
	{
		geo_index = 0;
	}
	else if (position.x == target.x && position.y == target.y)
	{
		geo_index = 0;
	}
	else if (position.y == 0)
	{
		geo_index = position.x * 16807u;
	}
	else if (position.x == 0)
	{
		geo_index = position.y * 48271u;
	}
	else
	{
		const ErosionType ly = GetErosionLevel(erosion_levels, { position.x, position.y - 1u }, target, depth);
		const ErosionType lx = GetErosionLevel(erosion_levels, { position.x - 1u, position.y }, target, depth);
		geo_index = ly * lx;
	}

	// compute the erosion level result
	ErosionType result = (geo_index + depth) % 20183u;
	erosion_levels[position] = result;
	return result;
}

RegionType GetRegionType(ErosionMap &erosion_levels, Position const &position, Position const &target, uint32_t depth)
{
	return RegionType(GetErosionLevel(erosion_levels, position, target, depth) % ErosionType(RegionType::Count));
}

void Part1(ErosionMap &erosion_levels, Position const &target, uint32_t depth)
{
	int risk = 0;
#if DEBUG_PRINT
	const char display_for_type[int(RegionType::Count)] =
	{
		'.',	// RegionType::Rocky
		'=',	// RegionType::Wet
		'|',	// RegionType::Narrow
	};
#endif
	const int risk_for_type[int(RegionType::Count)]
	{
		0,	// RegionType::Rocky
		1,	// RegionType::Wet
		2,	// RegionType::Narrow
	};
	for (CoordinateType y = 0; y <= target.y; ++y)
	{
		for (CoordinateType x = 0; x <= target.x; ++x)
		{
			const ErosionType erosion_level = GetErosionLevel(erosion_levels, { x, y }, target, depth);
			const RegionType type = RegionType(erosion_level % ErosionType(RegionType::Count));
#if DEBUG_PRINT
			std::cout << display_for_type[int(type)];
#endif
			risk += risk_for_type[int(type)];
		}
#if DEBUG_PRINT
		std::cout << '\n';
#endif
	}

	std::cout << "Part 1: total risk is " << risk << "\n";
}

void Part2(ErosionMap &erosion_levels, Position const &target, uint32_t depth)
{
	const CostType move_cost = 1;
	const CostType switch_cost = 7;

	const size_t reserve = 16 * (target.y + 1) * (target.x + 1);
#if DEBUG_PRINT
	std::unordered_map<StateType, StateType> came_from;
	came_from.reserve(reserve);
#endif
	std::unordered_map<StateType, CostType> g_score;
	g_score.reserve(reserve);
	std::unordered_map<StateType, CostType> f_score;
	f_score.reserve(reserve);
	std::unordered_set<StateType> open_set;
	open_set.reserve(reserve);
	std::unordered_set<StateType> closed_set;
	closed_set.reserve(reserve);

	// predicate comparing two nodes
	auto OpenQueuePredicate = [&](StateType const &a, StateType const &b)
	{
		const CostType a_f = f_score[a];
		const CostType b_f = f_score[b];
		if (a_f > b_f)
			return true;
		if (a_f < b_f)
			return false;
		return a > b;
	};

	// distance metric
	auto Distance = [&](StateType const &a, StateType const &b) -> CostType
	{
		return
			move_cost * (abs(a.first.x - b.first.x) + abs(a.first.y - b.first.y)) +
			switch_cost * (a.second != b.second);
	};

	std::vector<StateType> open_queue;
	StateType start({ 0, 0 }, ToolType::Torch);
	StateType goal(target, ToolType::Torch);
	open_queue.push_back(start);
	open_set.insert(start);
	g_score[start] = 0;
	f_score[start] = Distance(start, goal);

	std::vector<StateType> neighbors;
	neighbors.reserve(5);

	// add neighbor if compatible with current equipment
	auto GenerateNeighborsMove = [&](Position const &position, ToolType tool)
	{
		RegionType region = GetRegionType(erosion_levels, position, target, depth);
		if (uint8_t(region) != uint8_t(tool))
			neighbors.emplace_back(position, tool);
	};

	// switch tool if compatible with current location
	auto GenerateNeighborsSwitch = [&](Position const &position, ToolType tool)
	{
		RegionType region = GetRegionType(erosion_levels, position, target, depth);
		ToolType t1 = ToolType((uint8_t(tool) + 1) % uint8_t(ToolType::Count));
		if (uint8_t(region) != uint8_t(t1))
			neighbors.emplace_back(position, t1);
		ToolType t2 = ToolType((uint8_t(tool) + 2) % uint8_t(ToolType::Count));
		if (uint8_t(region) != uint8_t(t2))
			neighbors.emplace_back(position, t2);
	};

	uint32_t iterations = 0;

	while (!open_queue.empty())
	{
		// get the node in the open set with the best f_score
		std::pop_heap(open_queue.begin(), open_queue.end(), OpenQueuePredicate);
		const StateType current = open_queue.back();
		open_queue.pop_back();

		// if it's the goal, we're done!
		if (current == goal)
			break;

		++iterations;

		// close current node
		open_set.erase(current);
		closed_set.insert(current);

		// cost from the start to the current node
		const CostType current_g_score = g_score[current];

		// get the set of candidate neighbors
		neighbors.clear();
		if (current.first.y > 0)
			GenerateNeighborsMove({ current.first.x, current.first.y - 1u }, current.second);	// go north
		if (current.first.x > 0)
			GenerateNeighborsMove({ current.first.x - 1u, current.first.y }, current.second);	// go west
		GenerateNeighborsMove({ current.first.x + 1u, current.first.y }, current.second);	// go east
		GenerateNeighborsMove({ current.first.x, current.first.y + 1u }, current.second);	// go south
		GenerateNeighborsSwitch(current.first, current.second);

		// for each neighbor...
		for (StateType const &neighbor : neighbors)
		{
			// skip nodes that are closed
			if (closed_set.find(neighbor) != closed_set.end())
				continue;

			// distance from the start to this neighbor
			const CostType tentative_g_score = current_g_score + Distance(current, neighbor);

			// if the neighbor is new or the g score is better...
			const bool is_new = open_set.find(neighbor) == open_set.end();
			if (is_new || tentative_g_score < g_score[neighbor])
			{
				// update the best path
#if DEBUG_PRINT
				came_from[neighbor] = current;
#endif
				g_score[neighbor] = tentative_g_score;
				f_score[neighbor] = tentative_g_score + Distance(neighbor, goal);

				if (is_new)
				{
					// add the new node to the open queue
					open_set.insert(neighbor);
					open_queue.push_back(neighbor);
					std::push_heap(open_queue.begin(), open_queue.end(), OpenQueuePredicate);
				}
				else
				{
					// refresh the open queue because the order may have changed
					std::make_heap(open_queue.begin(), open_queue.end(), OpenQueuePredicate);
				}
			}
		}
	}

	std::cout << "Path planning took " << iterations << " iterations\n";

	// total distance to the goal
	CostType total_distance = g_score[goal];

#if DEBUG_PRINT
	// print out the path taken
	const char *region_name[int(RegionType::Count)] = { "rocky", "wet", "narrow" };
	const char *equip_name[int(ToolType::Count)] = { "neither", "torch", "climbing" };
	StateType state = goal;
	CostType state_cost = g_score[state];
	for (;;)
	{
		std::unordered_map<StateType, StateType>::const_iterator came_from_itor = came_from.find(state);
		if (came_from_itor == came_from.end())
			break;
		int from_cost = g_score[came_from_itor->second];
		const char *state_display = region_name[int(GetRegionType(erosion_levels, state.first, target, depth))];
		std::cout << state_display << " @(" << state.first.x << "," << state.first.y << ") " << equip_name[int(state.second)] << " cost=" << state_cost - from_cost << "\n";
		state = came_from_itor->second;
		state_cost = from_cost;
	}
	const char *state_display = region_name[int(GetRegionType(erosion_levels, state.first, target, depth))];
	std::cout << state_display << " @(" << state.first.x << "," << state.first.y << ") " << " " << equip_name[int(state.second)] << "\n";
#endif

	std::cout << "Part 2: best time to the goal is " << total_distance << "\n";
}

int main()
{
	uint32_t depth = 0u;
	Position target = { 0u, 0u };
	char line[256];
	if (std::cin.getline(line, sizeof(line)))
		sscanf_s(line, "depth: %u", &depth);
	if (std::cin.getline(line, sizeof(line)))
		sscanf_s(line, "target: %hu,%hu", &target.x, &target.y);

	// map is unbounded along the +X and +Y directions
	ErosionMap erosion_levels;

	Part1(erosion_levels, target, depth);
	Part2(erosion_levels, target, depth);

	return 0;
}
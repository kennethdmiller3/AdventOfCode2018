#include <iostream>
#include <vector>
#include <unordered_set>
#include <string>
#include <algorithm>

// https://adventofcode.com/2018/day/20

struct Point
{
	int16_t x;
	int16_t y;
};

bool operator<(Point const &a, Point const &b)
{
	if (a.y < b.y)
		return true;
	if (a.y > b.y)
		return false;
	return a.x < b.x;
}

bool operator==(Point const &a, Point const &b)
{
	return a.x == b.x && a.y == b.y;
}

namespace std
{
	template<>
	struct hash<Point>
	{
		inline std::size_t operator()(Point const &p) const
		{
			return hash<int32_t>()((int32_t(p.y) << 16) + p.x);
		}
	};
}

struct Rect
{
	Point min;
	Point max;
};

Rect ComputeBounds(std::string const &sequence)
{
	Rect bounds = { { 0, 0 }, { 0, 0 } };

	// state snapshots
	using ActiveSet = std::unordered_set<Point>;
	std::vector<ActiveSet> stack;
	ActiveSet active_set;

	// starting point
	const Point start = { 0, 0 };
	active_set.insert(start);

	// evaluate the sequence
	for (const char c : sequence)
	{
		switch (c)
		{
		case '^':
			break;
		case '$':
			break;
		case 'W':
			{
				ActiveSet new_set;
				for (Point cur : active_set)
				{
					--cur.x;
					if (bounds.min.x > cur.x)
						bounds.min.x = cur.x;
					new_set.insert(cur);
				}
				active_set = std::move(new_set);
			}
			break;
		case 'E':
			{
				ActiveSet new_set;
				for (Point cur : active_set)
				{
					++cur.x;
					if (bounds.max.x < cur.x)
						bounds.max.x = cur.x;
					new_set.insert(cur);
				}
				active_set = std::move(new_set);
			}
			break;
		case 'N':
			{
				ActiveSet new_set;
				for (Point cur : active_set)
				{
					--cur.y;
					if (bounds.min.y > cur.y)
						bounds.min.y = cur.y;
					new_set.insert(cur);
				}
				active_set = std::move(new_set);
			}
			break;
		case 'S':
			{
				ActiveSet new_set;
				for (Point cur : active_set)
				{
					++cur.y;
					if (bounds.max.y < cur.y)
						bounds.max.y = cur.y;
					new_set.insert(cur);
				}
				active_set = std::move(new_set);
			}
			break;
		case '(':
			// start a new group
			stack.push_back(ActiveSet());	// snapshot of ending state
			stack.push_back(active_set);	// snapshot of starting state
			break;
		case '|':
			// next alternative in the group
			for (Point cur : active_set)	// merge previous alternative result into ending state
				stack[stack.size() - 2].insert(cur);
			active_set = stack.back();		// restore starting state
			break;
		case ')':
			// finish the current group
			for (Point cur : active_set)	// merge previous alternative result into ending state
				stack[stack.size() - 2].insert(cur);
			stack.pop_back();
			active_set = stack.back();		// use merged ending state
			stack.pop_back();
			break;
		}
	}

	return bounds;
}

void PrintLayout(std::string const &sequence, Rect const &bounds)
{
	// dimensions of the layout
	int width = (bounds.max.x - bounds.min.x) * 2 + 3 + 1;
	int height = (bounds.max.y - bounds.min.y) * 2 + 3;

	// create and fill the layout
	std::string layout(width * height, '#');
	for (int y = 0; y < height; ++y)
		layout[y * width + width - 1] = '\n';

	// state snapshots
	using ActiveSet = std::unordered_set<Point>;
	std::vector<ActiveSet> stack;
	ActiveSet active_set;

	// starting point
	const Point start = { 1 - 2 * bounds.min.x, 1 - 2 * bounds.min.y };
	layout[start.y * width + start.x] = 'X';
	active_set.insert(start);

	// evaluate the sequence
	for (const char c : sequence)
	{
		switch (c)
		{
		case '^':
			break;
		case '$':
			break;
		case 'W':
			{
				ActiveSet new_set;
				for (Point cur : active_set)
				{
					--cur.x;
					layout[cur.y * width + cur.x] = '|';
					--cur.x;
					layout[cur.y * width + cur.x] = '.';
					new_set.insert(cur);
				}
				active_set = std::move(new_set);
			}
			break;
		case 'E':
			{
				ActiveSet new_set;
				for (Point cur : active_set)
				{
					++cur.x;
					layout[cur.y * width + cur.x] = '|';
					++cur.x;
					layout[cur.y * width + cur.x] = '.';
					new_set.insert(cur);
				}
				active_set = std::move(new_set);
			}
			break;
		case 'N':
			{
				ActiveSet new_set;
				for (Point cur : active_set)
				{
					--cur.y;
					layout[cur.y * width + cur.x] = '-';
					--cur.y;
					layout[cur.y * width + cur.x] = '.';
					new_set.insert(cur);
				}
				active_set = std::move(new_set);
			}
			break;
		case 'S':
			{
				ActiveSet new_set;
				for (Point cur : active_set)
				{
					++cur.y;
					layout[cur.y * width + cur.x] = '-';
					++cur.y;
					layout[cur.y * width + cur.x] = '.';
					new_set.insert(cur);
				}
				active_set = std::move(new_set);
			}
			break;
		case '(':
			// start a new group
			stack.push_back(ActiveSet());	// snapshot of ending state
			stack.push_back(active_set);	// snapshot of starting state
			break;
		case '|':
			// next alternative in the group
			for (Point cur : active_set)	// merge previous alternative result into ending state
				stack[stack.size() - 2].insert(cur);
			active_set = stack.back();		// restore starting state
			break;
		case ')':
			// finish the current group
			for (Point cur : active_set)	// merge previous alternative result into ending state
				stack[stack.size() - 2].insert(cur);
			stack.pop_back();
			active_set = stack.back();		// use merged ending state
			stack.pop_back();
			break;
		}
	}
	while (!stack.empty());

	std::cout << layout;
}

void Solve(std::string const &sequence, Rect const &bounds)
{
	// maximum number of rooms in each direction
	const int width = bounds.max.x - bounds.min.x + 1;
	const int height = bounds.max.y - bounds.min.y + 1;
	const int size = width * height;

	// room connectivity (N=1, E=2, S=4, W=8)
	std::vector<uint8_t> connections(width * height, 0);

	// state snapshots
	using ActiveSet = std::unordered_set<Point>;
	std::vector<ActiveSet> stack;
	ActiveSet active_set;

	// starting location shifted so that the minimum room is 0
	const Point start = { -bounds.min.x, -bounds.min.y };
	active_set.insert(start);

	// evaluate the sequence
	for (const char c : sequence)
	{
		switch (c)
		{
		case '^':
			break;
		case '$':
			break;
		case 'W':
			{
				ActiveSet new_set;
				for (Point cur : active_set)
				{
					connections[cur.y * width + cur.x] |= (1 << 3);
					--cur.x;
					connections[cur.y * width + cur.x] |= (1 << 1);
					new_set.insert(cur);
				}
				active_set = std::move(new_set);
			}
			break;
		case 'E':
			{
				ActiveSet new_set;
				for (Point cur : active_set)
				{
					connections[cur.y * width + cur.x] |= (1 << 1);
					++cur.x;
					connections[cur.y * width + cur.x] |= (1 << 3);
					new_set.insert(cur);
				}
				active_set = std::move(new_set);
			}
			break;
		case 'N':
			{
				ActiveSet new_set;
				for (Point cur : active_set)
				{
					connections[cur.y * width + cur.x] |= (1 << 0);
					--cur.y;
					connections[cur.y * width + cur.x] |= (1 << 2);
					new_set.insert(cur);
				}
				active_set = std::move(new_set);
			}
			break;
		case 'S':
			{
				ActiveSet new_set;
				for (Point cur : active_set)
				{
					connections[cur.y * width + cur.x] |= (1 << 2);
					++cur.y;
					connections[cur.y * width + cur.x] |= (1 << 0);
					new_set.insert(cur);
				}
				active_set = std::move(new_set);
			}
			break;
		case '(':
			// start a new group
			stack.push_back(ActiveSet());	// snapshot of ending state
			stack.push_back(active_set);	// snapshot of starting state
			break;
		case '|':
			// next alternative in the group
			for (Point cur : active_set)	// merge previous alternative result into ending state
				stack[stack.size() - 2].insert(cur);
			active_set = stack.back();		// restore starting state
			break;
		case ')':
			// finish the current group
			for (Point cur : active_set)	// merge previous alternative result into ending state
				stack[stack.size() - 2].insert(cur);
			stack.pop_back();
			active_set = stack.back();		// use merged ending state
			stack.pop_back();
			break;
		}
	}

	// shortest distance to each room
	std::vector<int> distance(width * height, INT_MAX);

	auto OpenQueuePredicate = [&](Point const &a, Point const &b) -> bool
	{
		int a_dist = distance[a.y * width + a.x];
		int b_dist = distance[b.y * width + b.x];
		if (b_dist < a_dist)
			return true;
		if (a_dist < b_dist)
			return false;
		return b < a;
	};

	std::vector<Point> open_queue;
	open_queue.push_back(start);

	distance[start.y * width + start.x] = 0;

	auto TryAddNeighbor = [&](Point const &neighbor, int new_dist)
	{
		if (new_dist < distance[neighbor.y * width + neighbor.x])
		{
			distance[neighbor.y * width + neighbor.x] = new_dist;
			open_queue.push_back(neighbor);
			std::push_heap(open_queue.begin(), open_queue.end(), OpenQueuePredicate);
		}
	};

	while (!open_queue.empty())
	{
		std::pop_heap(open_queue.begin(), open_queue.end(), OpenQueuePredicate);
		const Point cur = open_queue.back();
		open_queue.pop_back();

		const int cur_dist = distance[cur.y * width + cur.x];
		const int new_dist = cur_dist + 1;

		const uint8_t connect = connections[cur.y * width + cur.x];
		if (connect & (1 << 0))
			TryAddNeighbor({ cur.x, cur.y - 1 }, new_dist);
		if (connect & (1 << 1))
			TryAddNeighbor({ cur.x + 1, cur.y }, new_dist);
		if (connect & (1 << 2))
			TryAddNeighbor({ cur.x, cur.y + 1 }, new_dist);
		if (connect & (1 << 3))
			TryAddNeighbor({ cur.x - 1, cur.y }, new_dist);
	}

	// part 1: furthest distance
	int furthest = 0;
	for (int dist : distance)
		if (dist < INT_MAX && furthest < dist)
			furthest = dist;
	std::cout << "Part 1: largest distance is " << furthest << "\n";

	// part 2: rooms with distance 1000 or more
	int count = 0;
	for (int dist : distance)
		if (dist < INT_MAX && dist >= 1000)
			++count;
	std::cout << "Part 2: " << count << " rooms with a distance of at least a thousand\n";
}

int main()
{
	// read in the sequence
	std::string sequence;
	std::cin >> sequence;

	// determine the area bounds
	const Rect bounds = ComputeBounds(sequence);

	// solve the puzzle
	Solve(sequence, bounds);

	// print the layout for debugging purposes
	PrintLayout(sequence, bounds);

	return 0;
}
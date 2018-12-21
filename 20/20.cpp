#include <iostream>
#include <vector>
#include <string>

// https://adventofcode.com/2018/day/20

struct Point
{
	int x;
	int y;
};

struct Rect
{
	Point min;
	Point max;
};

Rect ComputeBounds(std::string const &sequence)
{
	Rect bounds = { { 0, 0 }, { 0, 0 } };

	// starting point
	const Point start = { 0, 0 };
	Point cur = start;

	// position stack
	std::vector<Point> stack;

	// evaluate the sequence
	for (const char c : sequence)
	{
		switch (c)
		{
		case '^':
			cur = start;
			break;
		case 'W':
			--cur.x;
			if (bounds.min.x > cur.x)
				bounds.min.x = cur.x;
			break;
		case 'E':
			++cur.x;
			if (bounds.max.x < cur.x)
				bounds.max.x = cur.x;
			break;
		case 'N':
			--cur.y;
			if (bounds.min.y > cur.y)
				bounds.min.y = cur.y;
			break;
		case 'S':
			++cur.y;
			if (bounds.max.y < cur.y)
				bounds.max.y = cur.y;
			break;
		case '(':
			stack.push_back(cur);
			break;
		case '|':
			cur = stack.back();
			break;
		case ')':
			cur = stack.back();
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

	// set starting location
	const int start = (1 - bounds.min.y * 2) * width + (1 - bounds.min.x * 2);
	int cur = start;
	layout[cur] = 'X';

	// position stack
	std::vector<int> stack;

	// evaluate the sequence
	for (const char c : sequence)
	{
		switch (c)
		{
		case '^':
			cur = start;
			break;
		case 'W':
			layout[cur -= 1] = '|';
			layout[cur -= 1] = '.';
			break;
		case 'E':
			layout[cur += 1] = '|';
			layout[cur += 1] = '.';
			break;
		case 'N':
			layout[cur -= width] = '-';
			layout[cur -= width] = '.';
			break;
		case 'S':
			layout[cur += width] = '-';
			layout[cur += width] = '.';
			break;
		case '(':
			stack.push_back(cur);
			break;
		case '|':
			cur = stack.back();
			break;
		case ')':
			cur = stack.back();
			stack.pop_back();
			break;
		}
	}

	std::cout << layout;
}

void Solve(std::string const &sequence, Rect const &bounds)
{
	struct State
	{
		int index;
		int dist;
	};

	// maximum number of rooms in each direction
	const int width = bounds.max.x - bounds.min.x + 1;
	const int height = bounds.max.y - bounds.min.y + 1;

	// starting location shifted so that the minimum room is 0
	const int start = (-bounds.min.y) * width + (-bounds.min.x);

	// shortest path to each room
	std::vector<int> min_dist(width * height, INT_MAX);

	// current state
	State cur = { start, 0 };

	// state stack
	std::vector<State> stack;

	// evaluate the sequence
	for (const char c : sequence)
	{
		switch (c)
		{
		case '^':
			cur = { start, 0 };
			break;
		case 'W':
			--cur.index;
			++cur.dist;
			if (min_dist[cur.index] > cur.dist)
				min_dist[cur.index] = cur.dist;
			break;
		case 'E':
			++cur.index;
			++cur.dist;
			if (min_dist[cur.index] > cur.dist)
				min_dist[cur.index] = cur.dist;
			break;
		case 'N':
			cur.index -= width;
			++cur.dist;
			if (min_dist[cur.index] > cur.dist)
				min_dist[cur.index] = cur.dist;
			break;
		case 'S':
			cur.index += width;
			++cur.dist;
			if (min_dist[cur.index] > cur.dist)
				min_dist[cur.index] = cur.dist;
			break;
		case '(':
			stack.push_back(cur);
			break;
		case '|':
			cur = stack.back();
			break;
		case ')':
			cur = stack.back();
			stack.pop_back();
			break;
		}
	}

	// part 1: furthest distance
	int furthest = 0;
	for (int dist : min_dist)
		if (dist < INT_MAX && furthest < dist)
			furthest = dist;
	std::cout << "Part 1: largest distance is " << furthest << "\n";

	// part 2: rooms with distance 1000 or more
	int count = 0;
	for (int dist : min_dist)
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
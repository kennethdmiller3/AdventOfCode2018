#include <iostream>
#include <vector>
#include <unordered_set>
#include <algorithm>

// https://adventofcode.com/2018/day/25

// fixed point in spacetime
struct Point
{
	int x;
	int y;
	int z;
	int t;
};

// read the list of points
void ReadPoints(std::vector<Point> &output, std::istream &input)
{
	char line[256];
	while (input.getline(line, sizeof(line)))
	{
		Point point;
		if (sscanf_s(line, "%d,%d,%d,%d", &point.x, &point.y, &point.z, &point.t) == 4)
			output.push_back(point);
	}
}

// Manhattan distance between points
int ManhattanDistance(Point const &a, Point const &b)
{
	return abs(a.x - b.x) + abs(a.y - b.y) + abs(a.z - b.z) + abs(a.t - b.t);
}

// PART 1 (...the only part)
void Part1(std::vector<Point> const &points)
{
	const int d = 3;

	const int count = int(points.size());

	// which constellation each point is assigned to
	// (not actually used for anything, but hey, why not)
	std::vector<int> constellation_for_point(count, -1);

	// current constellation count
	int constellation_count = 0;

	// set of points needing a constellation assigned
	std::unordered_set<int> points_to_assign;
	for (int i = 0; i < count; ++i)
		points_to_assign.insert(i);

	// set of open nodes to process
	struct Node
	{
		int point_index;
		int constellation_index;
	};
	std::vector<Node> open_set;

	// while there are still points to assign...
	while (!points_to_assign.empty())
	{
		// grab the first point from the points to assign
		int first = *points_to_assign.begin();
		points_to_assign.erase(first);

		// add it to the open set
		open_set.push_back({ first, constellation_count++ });

		// while processing the graph of points connected to the constellation...
		while (!open_set.empty())
		{
			// get the next node
			Node current = open_set.back();
			open_set.pop_back();

			// list of points to remove
			// (since removing points during iteration is hazardous)
			std::vector<int> points_to_remove;

			// for each remaining point...
			for (int other : points_to_assign)
			{
				// if it's close enough to the current point
				if (ManhattanDistance(points[current.point_index], points[other]) <= d)
				{
					// mark the point for removal
					points_to_remove.push_back(other);

					// assign the point's constellation
					constellation_for_point[other] = current.constellation_index;

					// add to the open set
					open_set.push_back({ other, current.constellation_index });
				}
			}

			// now remove the points
			for (int remove : points_to_remove)
			{
				points_to_assign.erase(remove);
			}
		}
	}

	// print the result
	std::cout << "Part 1: " << constellation_count << " constellations\n";
}

int main()
{
	std::vector<Point> points;
	ReadPoints(points, std::cin);

	Part1(points);

	return 0;
}
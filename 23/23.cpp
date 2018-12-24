#include <iostream>
#include <vector>
#include <algorithm>

// 3D point
struct Point
{
	int x;
	int y;
	int z;
};

// point equality
bool operator==(Point const &a, Point const &b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

// axis-aligned bounding box
struct Box
{
	Point min;
	Point max;
};

// nanobot representation
struct Nanobot
{
	Point pos;
	int r;
};

// read in the positions and ranges of nanobots
void ReadNanobots(std::vector<Nanobot> &output, std::istream &input)
{
	char line[256];
	while (input.getline(line, sizeof(line)))
	{
		Nanobot nanobot;
		if (sscanf_s(line, "pos=<%d,%d,%d>, r=%d", &nanobot.pos.x, &nanobot.pos.y, &nanobot.pos.z, &nanobot.r) == 4)
			output.push_back(nanobot);
	}
}

// PART 1
void Part1(std::vector<Nanobot> const &nanobots)
{
	// find the nanobot with the largest range
	int best_radius = -1;
	int best_index = -1;
	for (int i = 0; i < int(nanobots.size()); ++i)
	{
		Nanobot const &nanobot = nanobots[i];
		if (best_radius < nanobot.r)
		{
			best_radius = nanobot.r;
			best_index = i;
		}
	}
	Nanobot const &strongest = nanobots[best_index];
	std::cout << "Part 1: nanobot #" << best_index << " pos=<" << strongest.pos.x << "," << strongest.pos.y << "," << strongest.pos.z << "> r=" << strongest.r << "\n";

	// now count the number of nanobots within its range
	int count = 0;
	for (int i = 0; i < int(nanobots.size()); ++i)
	{
		Nanobot const &nanobot = nanobots[i];
		if (abs(nanobot.pos.x - strongest.pos.x) + abs(nanobot.pos.y - strongest.pos.y) + abs(nanobot.pos.z - strongest.pos.z) <= strongest.r)
			++count;
	}

	std::cout << "Part 1: " << count << " nanobots in range\n";
}

// test for overlap between nanobot coverage and bounding box
bool DoesNanobotOverlapBox(Nanobot const &nanobot, Box const &bounds)
{
	if (bounds.min == bounds.max)
	{
		// simple case: bounds cover a single voxel
		return abs(nanobot.pos.x - bounds.min.x) + abs(nanobot.pos.y - bounds.min.y) + abs(nanobot.pos.z - bounds.min.z) <= nanobot.r;
	}

	const Point nanobot_pos_2 = { nanobot.pos.x * 2, nanobot.pos.y * 2, nanobot.pos.z * 2 };
	const int nanobot_r_2 = nanobot.r * 2;

	const Point bounds_mid_2 = { bounds.max.x + bounds.min.x, bounds.max.y + bounds.min.y, bounds.max.z + bounds.min.z };
	const Point bounds_ext_2 = { bounds.max.x - bounds.min.x, bounds.max.y - bounds.min.y, bounds.max.z - bounds.min.z };

	// axes to test
	const Point axes[] =
	{
		// box faces
		{ 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 },
		// octahedron faces
		{ 1, 1, 0 }, { -1, 1, 0 }, { 1, 0, 1 }, { -1, 0, 1 }, { 0, 1, 1 }, { 0, -1, 1 },
		// edge cross products
		{ 1, 1, 1 }, { -1, 1, 1}, { 1, -1, 1 }, { -1, -1, 1 }
	};

	// test separation along each axis
	for (Point const &axis : axes)
	{
		const int dist_2 = 
			abs(
			axis.x * (nanobot_pos_2.x - bounds_mid_2.x) +
			axis.y * (nanobot_pos_2.y - bounds_mid_2.y) +
			axis.z * (nanobot_pos_2.z - bounds_mid_2.z)
			);
		const int test_2 = 
			nanobot_r_2 +
			abs(axis.x * bounds_ext_2.x) + 
			abs(axis.y * bounds_ext_2.y) + 
			abs(axis.z * bounds_ext_2.z);
		if (dist_2 > test_2)
			return false;
	}
	return true;
}

int CountNanobotsOverlappingBox(std::vector<Nanobot> const &nanobots, Box const &bounds)
{
	int count = 0;
	for (Nanobot const &nanobot : nanobots)
		count += DoesNanobotOverlapBox(nanobot, bounds);
	return count;
}

// PART 2
void Part2(std::vector<Nanobot> const &nanobots)
{
	// compute the bounding box of the positions
	Box bounds = { { INT_MAX, INT_MAX, INT_MAX }, { INT_MIN, INT_MIN, INT_MIN } };
	for (int i = 0; i < int(nanobots.size()); ++i)
	{
		Nanobot const &nanobot = nanobots[i];
		if (bounds.min.x > nanobot.pos.x)
			bounds.min.x = nanobot.pos.x;
		if (bounds.min.y > nanobot.pos.y)
			bounds.min.y = nanobot.pos.y;
		if (bounds.min.z > nanobot.pos.z)
			bounds.min.z = nanobot.pos.z;
		if (bounds.max.x < nanobot.pos.x)
			bounds.max.x = nanobot.pos.x;
		if (bounds.max.y < nanobot.pos.y)
			bounds.max.y = nanobot.pos.y;
		if (bounds.max.z < nanobot.pos.z)
			bounds.max.z = nanobot.pos.z;
	}
#if DEBUG_PRINT
	std::cout << "Part 2: bounds min=<" << bounds.min.x << "," << bounds.min.y << "," << bounds.min.z << "> max=<" << bounds.max.x << "," << bounds.max.y << "," << bounds.max.z << ">\n";
#endif

	// priority queue of bounding boxes
	struct QueueEntry
	{
		Box box;
		int overlaps;
	};
	std::vector<QueueEntry> open_queue;

	// compare bounding boxes
	auto OpenQueuePredicate = [&](QueueEntry const &a, QueueEntry const &b)
	{
		// first compare by number of overlaps
		if (a.overlaps < b.overlaps)
			return true;
		if (a.overlaps > b.overlaps)
			return false;

		// then compare by Manhattan distance from the origin (0, 0, 0)
		const int a_dist0 =
			(a.box.min.x >= 0 ? a.box.min.x : (a.box.max.x <= 0 ? -a.box.max.x : 0)) +
			(a.box.min.y >= 0 ? a.box.min.y : (a.box.max.y <= 0 ? -a.box.max.y : 0)) +
			(a.box.min.z >= 0 ? a.box.min.z : (a.box.max.z <= 0 ? -a.box.max.z : 0));
		const int b_dist0 =
			(b.box.min.x >= 0 ? b.box.min.x : (b.box.max.x <= 0 ? -b.box.max.x : 0)) +
			(b.box.min.y >= 0 ? b.box.min.y : (b.box.max.y <= 0 ? -b.box.max.y : 0)) +
			(b.box.min.z >= 0 ? b.box.min.z : (b.box.max.z <= 0 ? -b.box.max.z : 0));
		return a_dist0 > b_dist0;
	};

	// result so far
	QueueEntry result = { { { 0, 0, 0 }, { 0, 0, 0 } }, 0 };

	// start with the bounds
	open_queue.push_back({ bounds, int(nanobots.size()) });

	// while there are boxes to process...
	while (!open_queue.empty())
	{
		// get the next box
		std::pop_heap(open_queue.begin(), open_queue.end(), OpenQueuePredicate);
		const QueueEntry current = open_queue.back();
		open_queue.pop_back();

		// don't bother subdividing if the count isn't better better than the result so far
		if (current.overlaps <= result.overlaps)
			continue;

		// if the current box is a single voxel...
		if (current.box.min == current.box.max)
		{
			// update the result
			if (result.overlaps < current.overlaps)
			{
				result = current;
			}
#if DEBUG_PRINT
			std::cout << "pos=<" << current.min.x << "," << current.min.y << "," << current.min.z << "> overlaps=" << current_count << " best=" << result_count << "\n";
#endif
			continue;
		}

#if DEBUG_PRINT
		std::cout << "min=<" << current.min.x << "," << current.min.y << "," << current.min.z << "> max=" << current.max.x << "," << current.max.y << "," << current.max.z << "> count=" << current_count << "\n";
#endif

		// subdivide the current box
		const Point current_box_mid = {
			(current.box.max.x + current.box.min.x + 1) / 2,
			(current.box.max.y + current.box.min.y + 1) / 2,
			(current.box.max.z + current.box.min.z + 1) / 2,
		};
		const Box subdivides[8] = {
			{ { current.box.min.x, current.box.min.y, current.box.min.z }, { current_box_mid.x - 1, current_box_mid.y - 1, current_box_mid.z - 1 } },
			{ { current_box_mid.x, current.box.min.y, current.box.min.z }, { current.box.max.x    , current_box_mid.y - 1, current_box_mid.z - 1 } },
			{ { current.box.min.x, current_box_mid.y, current.box.min.z }, { current_box_mid.x - 1, current.box.max.y    , current_box_mid.z - 1 } },
			{ { current_box_mid.x, current_box_mid.y, current.box.min.z }, { current.box.max.x    , current.box.max.y    , current_box_mid.z - 1 } },
			{ { current.box.min.x, current.box.min.y, current_box_mid.z }, { current_box_mid.x - 1, current_box_mid.y - 1, current.box.max.z     } },
			{ { current_box_mid.x, current.box.min.y, current_box_mid.z }, { current.box.max.x    , current_box_mid.y - 1, current.box.max.z     } },
			{ { current.box.min.x, current_box_mid.y, current_box_mid.z }, { current_box_mid.x - 1, current.box.max.y    , current.box.max.z     } },
			{ { current_box_mid.x, current_box_mid.y, current_box_mid.z }, { current.box.max.x    , current.box.max.y    , current.box.max.z     } },
		};
		for (Box const &subdivide : subdivides)
		{
			// if the subdivision box is not degenerate...
			if (subdivide.min.x <= subdivide.max.x && subdivide.min.y <= subdivide.max.y && subdivide.min.z <= subdivide.max.x)
			{
				// count the number of overlaps for the subdivision box
				const int subdivide_count = CountNanobotsOverlappingBox(nanobots, subdivide);

				// if potentially a better result
				if (subdivide_count > 0 && subdivide_count >= result.overlaps)
				{
					// queue the subdivision box
					open_queue.push_back({ subdivide, subdivide_count });
					std::push_heap(open_queue.begin(), open_queue.end(), OpenQueuePredicate);
				}
			}
		}
	}

	std::cout << "Part 2: point at <" << result.box.min.x << "," << result.box.min.y << "," << result.box.min.z << "> overlaps " << result.overlaps << "\n";

	const int result_dist = abs(result.box.min.x) + abs(result.box.min.y) + abs(result.box.min.z);
	std::cout << "Part 2: Manhattan distance from <0,0,0> is " << result_dist << "\n";
}

int main()
{
	// read the nanobot positions and ranges
	std::vector<Nanobot> nanobots;
	ReadNanobots(nanobots, std::cin);

	Part1(nanobots);
	Part2(nanobots);

	return 0;
}
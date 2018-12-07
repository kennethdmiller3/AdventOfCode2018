#include <iostream>
#include <vector>
#include <algorithm>

// https://adventofcode.com/2018/day/6

struct Coordinate
{
	short x;
	short y;
};

// read in the coordinates
void ReadCoordinates(std::vector<Coordinate> &output, std::istream &input)
{
	char line[256];
	while (input.getline(line, sizeof(line)))
	{
		Coordinate coordinate = { -1, -1 };
		if (sscanf_s(line, "%hd, %hd", &coordinate.x, &coordinate.y) == 2)
		{
			output.push_back(coordinate);
		}
	}
}

// PART 1
void Part1(std::vector<Coordinate> const &coordinates)
{
	// get the bounding box of the coordinates
	short minX = SHRT_MAX, minY = SHRT_MAX, maxX = SHRT_MIN, maxY = SHRT_MIN;
	for (Coordinate const &coordinate : coordinates)
	{
		if (minX > coordinate.x)
			minX = coordinate.x;
		if (maxX < coordinate.x)
			maxX = coordinate.x;
		if (minY > coordinate.y)
			minY = coordinate.y;
		if (maxY < coordinate.y)
			maxY = coordinate.y;
	}

	// pad by half the largest dimension since that's the furthest a Voronoi cell could extend
	const int pad = (std::max(maxX - minX, maxY - minY) + 1) / 2;
	minY -= pad;
	maxY += pad;
	minX -= pad;
	maxX += pad;

	// total area for each coordinate
	const int sizeX = maxX - minX + 1;
	const int sizeY = maxY - minY + 1;
	std::vector<int> area(coordinates.size());

	// find the closest coordinate for each location (creating a Voronoi diagram)
	for (int y = minY; y <= maxY; ++y)
	{
		for (int x = minX; x <= maxX; ++x)
		{
			// find the closest coordinate
			int bestIndex = -1;
			int bestDist = INT_MAX;
			for (int curIndex = 0; curIndex < coordinates.size(); ++curIndex)
			{
				// compute Manhattan distance
				Coordinate const &coordinate = coordinates[curIndex];
				const int curDist = abs(coordinate.x - x) + abs(coordinate.y - y);

				// update the best coordinate if the distance is smaller
				if (bestDist > curDist)
				{
					bestDist = curDist;
					bestIndex = curIndex;
				}
				// mark the location as conflicted if two coordinates have the same distance
				else if (bestDist == curDist && bestIndex != curIndex)
				{
					bestIndex = -1;
				}
			}

			// if the location is not conflicted...
			if (bestIndex >= 0)
			{
				// if the location is on the outer perimeter (representing "infinity")
				if (y == minY || y == maxY || x == minX || x == maxX)
				{
					// mark the area as infinite and thus invalid
					area[bestIndex] = -1;
				}
				// otherwise increment the area
				else if (area[bestIndex] >= 0)
				{
					++area[bestIndex];
				}
			}
		}
	}

	// find the coordinate with the largest finite area
	int bestIndex = -1;
	int bestArea = -1;
	for (int curIndex = 0; curIndex < area.size(); ++curIndex)
	{
		int curArea = area[curIndex];
		if (bestArea < curArea)
		{
			bestArea = curArea;
			bestIndex = curIndex;
		}
	}
	if (bestIndex >= 0)
	{
		Coordinate const &bestCoordinate = coordinates[bestIndex];
		std::cout << "Part 1: coordinate " << bestCoordinate.x << ", " << bestCoordinate.y << " has largest finite area " << bestArea << "\n";
	}
	else
	{
		std::cout << "Part 1: no suitable area found :(\n";
	}
}

// PART 2
void Part2(std::vector<Coordinate> const &coordinates)
{
	// find bounds;
	short minX = SHRT_MAX, minY = SHRT_MAX, maxX = SHRT_MIN, maxY = SHRT_MIN;
	for (Coordinate const &coordinate : coordinates)
	{
		if (minX > coordinate.x)
			minX = coordinate.x;
		if (maxX < coordinate.x)
			maxX = coordinate.x;
		if (minY > coordinate.y)
			minY = coordinate.y;
		if (maxY < coordinate.y)
			maxY = coordinate.y;
	}

	// find the locations whose total distance to all coordinates is less than 10000
	int totalArea = 0;
	for (int y = minY; y <= maxY; ++y)
	{
		for (int x = minX; x <= maxX; ++x)
		{
			// measure the total distance to all coordinates
			int totalDist = 0;
			int curIndex;
			for (curIndex = 0; curIndex < coordinates.size(); ++curIndex)
			{
				// sum Manhattan distance
				Coordinate const &coordinate = coordinates[curIndex];
				totalDist += abs(x - coordinate.x) + abs(y - coordinate.y);

				// quit if the total distance is too large
				if (totalDist >= 10000)
					break;
			}

			// increment the area if the total distance was not too large
			if (curIndex == coordinates.size())
			{
				++totalArea;
			}
		}
	}
	std::cout << "Part 2: " << totalArea << " locations have a total distance less than 10000\n";
}

int main()
{
	// read in the coordinates
	std::vector<Coordinate> coordinates;
	ReadCoordinates(coordinates, std::cin);

	Part1(coordinates);
	Part2(coordinates);

	return 0;
}
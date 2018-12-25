#include <iostream>
#include <vector>

// https://adventofcode.com/2018/day/10

struct Coordinate
{
	int positionX;
	int positionY;
	int velocityX;
	int velocityY;
};

// read the coordinates
void ReadCoordinates(std::vector<Coordinate> &output, std::istream &input)
{
	char line[256];
	while (input.getline(line, sizeof(line)))
	{
		Coordinate coordinate;
		sscanf_s(line, "position=<%d, %d> velocity=<%d, %d>", &coordinate.positionX, &coordinate.positionY, &coordinate.velocityX, &coordinate.velocityY);
		output.push_back(coordinate);
	}
}

void Part1And2(std::vector<Coordinate> const &coordinates)
{
	// find the time at which the bounding box is smallest
	int bestTime = -1;
	int bestMinX = INT_MAX, bestMaxX = INT_MIN;
	int bestMinY = INT_MAX, bestMaxY = INT_MIN;
	int64_t bestArea = INT64_MAX;

	// for each time step
	int curTime = 0;
	for (;;)
	{
		// find the bounding box of the coordinates
		int curMinX = INT_MAX, curMaxX = INT_MIN;
		int curMinY = INT_MAX, curMaxY = INT_MIN;
		for (Coordinate const &coordinate : coordinates)
		{
			int x = coordinate.positionX + curTime * coordinate.velocityX;
			int y = coordinate.positionY + curTime * coordinate.velocityY;
			if (curMinX > x)
				curMinX = x;
			if (curMaxX < x)
				curMaxX = x;
			if (curMinY > y)
				curMinY = y;
			if (curMaxY < y)
				curMaxY = y;
		}

		// get the area of the bounding box
		int64_t curArea = int64_t(curMaxX - curMinX + 1) * int64_t(curMaxY - curMinY + 1);

		// update the best time
		if (bestArea > curArea)
		{
			bestTime = curTime;
			bestMinX = curMinX;
			bestMaxX = curMaxX;
			bestMinY = curMinY;
			bestMaxY = curMaxY;
			bestArea = curArea;
		}
		else
		{
			break;
		}

		++curTime;
	}

	// build the message output
	int outputWidth = (bestMaxX - bestMinX + 2);
	int outputHeight = (bestMaxY - bestMinY + 1);
	std::string output(outputWidth * outputHeight, ' ');
	for (int y = bestMinY; y <= bestMaxY; ++y)
	{
		output[(y - bestMinY) * outputWidth + outputWidth - 1] = '\n';
	}
	for (Coordinate const &coordinate : coordinates)
	{
		int x = coordinate.positionX + bestTime * coordinate.velocityX;
		int y = coordinate.positionY + bestTime * coordinate.velocityY;
		output[(y - bestMinY) * outputWidth + (x - bestMinX)] = '#';
	}

	std::cout << "Part 1: message is...\n";
	std::cout << output.c_str();

	std::cout << "Part 2: time is " << bestTime << "\n";
}

int main()
{
	std::vector<Coordinate> coordinates;
	ReadCoordinates(coordinates, std::cin);

	Part1And2(coordinates);

	return 0;
}
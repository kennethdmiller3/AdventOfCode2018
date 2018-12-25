#include <iostream>
#include <chrono>

// https://adventofcode.com/2018/day/11

int GetPower(int x, int y, int serial)
{
	int rackID = x + 10;
	int power = rackID * y;
	power += serial;
	power *= rackID;
	power /= 100;
	power %= 10;
	power -= 5;
	return power;
}

void GetBestPower(int &bestPower, int &bestX, int &bestY, int8_t const (&power)[300][300], int size)
{
	bestPower = INT_MIN, bestX = INT_MIN, bestY = INT_MIN;

	// compute partial sums of <size> successive power values along x;
	// values range from -1500 (-5*300) to 1200 (4*300) so use int16_t instead of int8_t
	int16_t partial[300][300] = { 0 };
	for (int y = 0; y < 300; ++y)
	{
		// sum of the <size> elements from power[y][0] to power[y][size-1]
		int16_t sum = power[y][0];
		for (int x = 1; x < size; ++x)
		{
			sum += power[y][x];
		}
		partial[y][0] = sum;

		for (int x = 1; x < 300 - size; ++x)
		{
			// sum of the <size> elements from power[y][x] to power[y][x+size-1]
			sum -= power[y][x - 1];
			sum += power[y][x - 1 + size];
			partial[y][x] = sum;
		}
	}

	// compute sums of <size> successive partial sums along y to get the total power in each <size>x<size> square
	for (int x = 0; x < 300 - size; ++x)
	{
		// sum of the <size> elements from partial[0][x] to partial[size-1][x]
		int32_t sum = partial[0][x];
		for (int y = 1; y < size; ++y)
		{
			sum += partial[y][x];
		}

		// update the best total power
		if (bestPower < sum)
		{
			bestPower = sum;
			bestX = x + 1;
			bestY = 1;
		}

		for (int y = 1; y < 300 - size; ++y)
		{
			// sum of the <size> elements from partial[y][x] to partial[y+size-1][x]
			sum -= partial[y - 1][x];
			sum += partial[y - 1 + size][x];

			// update the best total power
			if (bestPower < sum)
			{
				bestPower = sum;
				bestX = x + 1;
				bestY = y + 1;
			}
		}
	}
}

void Part1(int8_t const (&power)[300][300])
{
	// find the best 3x3 square
	int bestPower, bestX, bestY;
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	GetBestPower(bestPower, bestX, bestY, power, 3);
	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	std::cout << "Largest total power " << bestPower << " at (" << bestX << ", " << bestY << ") [" << std::chrono::duration<float>(t2 - t1).count() << "]\n";
}

void Part2(int8_t const (&power)[300][300])
{
	// find the best square of any size
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	int bestPower = INT_MIN, bestX = INT_MIN, bestY = INT_MIN, bestSize = INT_MIN;
	for (int size = 1; size <= 300; ++size)
	{
		// find the best <size>x<size> square
		int curPower, curX, curY;
		GetBestPower(curPower, curX, curY, power, size);
		if (bestPower < curPower)
		{
			bestPower = curPower;
			bestX = curX;
			bestY = curY;
			bestSize = size;
		}
	}
	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	std::cout << "Largest total power " << bestPower << " at (" << bestX << ", " << bestY << ") size " << bestSize << " [" << std::chrono::duration<float>(t2 - t1).count() << "]\n";
}

void GetBestPowerSAT(int &bestPower, int &bestX, int &bestY, int32_t const (&summed_area_table)[301][301], int size)
{
	bestPower = INT_MIN, bestX = INT_MIN, bestY = INT_MIN;

	for (int y = 0; y < 300 - size; ++y)
	{
		for (int x = 0; x < 300 - size; ++x)
		{
			// compute sum over the area
			int32_t sum =
				+ summed_area_table[y][x]
				- summed_area_table[y][x + size]
				+ summed_area_table[y + size][x + size]
				- summed_area_table[y + size][x];

			// update the best total power
			if (bestPower < sum)
			{
				bestPower = sum;
				bestX = x + 1;
				bestY = y + 1;
			}
		}
	}
}

void Part1SAT(int32_t const (&summed_area_table)[301][301])
{
	// find the best 3x3 square
	int bestPower, bestX, bestY;
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	GetBestPowerSAT(bestPower, bestX, bestY, summed_area_table, 3);
	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	std::cout << "Largest total power " << bestPower << " at (" << bestX << ", " << bestY << ") [" << std::chrono::duration<float>(t2 - t1).count() << "]\n";
}

void Part2SAT(int32_t const (&summed_area_table)[301][301])
{
	// find the best square of any size
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	int bestPower = INT_MIN, bestX = INT_MIN, bestY = INT_MIN, bestSize = INT_MIN;
	for (int size = 1; size <= 300; ++size)
	{
		// find the best <size>x<size> square
		int curPower, curX, curY;
		GetBestPowerSAT(curPower, curX, curY, summed_area_table, size);
		if (bestPower < curPower)
		{
			bestPower = curPower;
			bestX = curX;
			bestY = curY;
			bestSize = size;
		}
	}
	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	std::cout << "Largest total power " << bestPower << " at (" << bestX << ", " << bestY << ") size " << bestSize << " [" << std::chrono::duration<float>(t2 - t1).count() << "]\n";
}

int main()
{
	int serial;
	std::cin >> serial;

	// power level for each fuel cell
	// (values range from -5 to +4)
	int8_t power[300][300];
	for (int y = 0; y < 300; ++y)
	{
		for (int x = 0; x < 300; ++x)
		{
			power[y][x] = GetPower(x + 1, y + 1, serial);
		}
	}

	Part1(power);
	Part2(power);

	// build summed-area table (shifted by 1 in x and y)
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	int32_t summed_area_table[301][301] = { 0 };
	for (int y = 0; y < 300; ++y)
	{
		for (int x = 0; x < 300; ++x)
		{
			summed_area_table[y + 1][x + 1] = power[y][x];
			summed_area_table[y + 1][x + 1] += summed_area_table[y][x + 1];
			summed_area_table[y + 1][x + 1] -= summed_area_table[y][x];
			summed_area_table[y + 1][x + 1] += summed_area_table[y + 1][x];
		}
	}
	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	std::cout << "summed area table [" << std::chrono::duration<float>(t2 - t1).count() << "]\n";

	Part1SAT(summed_area_table);
	Part2SAT(summed_area_table);

	return 0;
}
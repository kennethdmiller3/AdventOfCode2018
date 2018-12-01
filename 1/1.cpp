#include <iostream>
#include <vector>
#include <unordered_set>

// https://adventofcode.com/2018/day/1

// read a list of frequency changes from the specified input stream
void ReadFrequencyChanges(std::vector<int> &output, std::istream &input)
{
	int change;
	while (input >> change)
	{
		output.push_back(change);
	}
}

// PART 1
void Part1(std::vector<int> const &changes)
{
	int frequency = 0;
	for (int change : changes)
	{
		frequency += change;
	}
	std::cout << "Part 1: final frequency " << frequency << "\n";
}

// PART 2
void Part2(std::vector<int> const &changes)
{
	std::unordered_set<int> frequencies;

	int frequency = 0;
	int iteration = 0;
	frequencies.insert(frequency);
	for (;;)
	{
		for (int change : changes)
		{
			frequency += change;
			++iteration;
			if (frequencies.find(frequency) != frequencies.end())
			{
				// found the frequency
				std::cout << "Part 2: repeated frequency " << frequency << " on iteration " << iteration << "\n";
				return;
			}
			frequencies.insert(frequency);
		}
	}
}

int main()
{
	std::vector<int> changes;
	ReadFrequencyChanges(changes, std::cin);

	Part1(changes);
	Part2(changes);

	return 0;
}
